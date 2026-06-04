/**
 * @file Board.h
 * @brief Declaração da classe Board para o jogo Bus Jam Puzzle.
 *
 * Board é o núcleo estrutural do domínio. Representa o tabuleiro completo:
 * dimensões, grelha de ocupação, e as três coleções de entidades (Bus,
 * Passenger, Platform). Não contém lógica de jogo — essa responsabilidade
 * pertence ao BoardEngine. Board é um repositório estrutural com consultas
 * ricas e mutações controladas.
 *
 * ### Responsabilidades de Board:
 *  - Gerir o espaço bidimensional da grelha (ocupação de células).
 *  - Fornecer acesso tipado às entidades por id ou posição.
 *  - Validar posições e detetar colisões.
 *  - Verificar condições de vitória.
 *  - Coordenar o reset completo de todas as entidades.
 *
 * ### O que Board NÃO faz:
 *  - Não decide se um movimento é legal (BoardEngine / MovementSolver).
 *  - Não executa transições de embarque (BoardEngine).
 *  - Não conhece QML, sinais Qt, nem animações.
 *
 * ### Modelo de posse de memória:
 *  Board possui as suas entidades via QList<std::unique_ptr<T>>.
 *  Os métodos de acesso devolvem ponteiros observadores (T*) — nunca
 *  transferem posse. O chamador nunca deve guardar estes ponteiros além
 *  do âmbito imediato; a Board pode reordenar ou limpar as listas.
 *
 * ### Grelha de ocupação (occupancyGrid_):
 *  Matriz rows × cols de OccupancyType. Cada célula regista o tipo
 *  de entidade que a ocupa (ou None). A grelha é a fonte de verdade
 *  para colisões; é mantida sincronizada com as posições das entidades.
 *
 * ### Invariantes da classe:
 *  - 0 < rows_ ≤ kMaxRows, 0 < cols_ ≤ kMaxCols
 *  - Para todo Bus b ativo: occupancyGrid_[b.row()][b.col()] == Bus
 *  - Para todo Passenger p Waiting: occupancyGrid_[p.row()][p.col()] == Passenger
 *  - Para toda Platform pl: occupancyGrid_[pl.row()][pl.col()] == Platform
 *  - Nenhuma célula tem mais do que uma entidade registada
 */

#pragma once

#include "Bus.h"
#include "Passenger.h"
#include "Platform.h"

#include <QString>
#include <QPoint>
#include <QVector>
#include <memory>    // std::unique_ptr
#include <optional>  // std::optional
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Tipos auxiliares locais
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Tipo de entidade que ocupa uma célula da grelha.
 *
 * Usado na occupancyGrid_ para consultas O(1) de colisão.
 * Manter como quint8 minimiza o footprint da grelha em memória.
 */
enum class OccupancyType : quint8
{
    None      = 0,  ///< Célula vazia.
    BusEntity = 1,  ///< Ocupada por um autocarro ativo.
    Passenger = 2,  ///< Ocupada por um passageiro em Waiting.
    Platform  = 3   ///< Ocupada por uma plataforma (posição estrutural fixa).
};

/**
 * @brief Resultado de uma consulta de célula por posição.
 *
 * Agrega o tipo de ocupação e o id da entidade numa única estrutura,
 * evitando chamadas duplas à grelha.
 */
struct CellInfo
{
    OccupancyType type  { OccupancyType::None };
    int           id    { -1 };  ///< Id da entidade; -1 se None.
};

// ─────────────────────────────────────────────────────────────────────────────
// Constantes do módulo
// ─────────────────────────────────────────────────────────────────────────────

namespace BoardConstants
{
    constexpr int kMinRows     =  4;
    constexpr int kMaxRows     = 20;
    constexpr int kMinCols     =  4;
    constexpr int kMaxCols     = 20;
    constexpr int kMinPlatforms =  4;
    constexpr int kMaxPlatforms =  8;
}

// ─────────────────────────────────────────────────────────────────────────────
// Classe Board
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Tabuleiro completo do Bus Jam Puzzle.
 *
 * Instância única por nível. Criada pelo LevelLoader com base na definição
 * JSON do nível e passada ao BoardEngine que a usa como repositório central.
 */
class Board
{
public:
    // ─── Construção e destruição ───────────────────────────────────────────

    /**
     * @brief Constrói um tabuleiro vazio com as dimensões dadas.
     *
     * Inicializa a grelha de ocupação com OccupancyType::None em todas
     * as células. As entidades são adicionadas depois via addBus(),
     * addPassenger() e addPlatform().
     *
     * @param rows Número de linhas. Deve estar em [kMinRows, kMaxRows].
     * @param cols Número de colunas. Deve estar em [kMinCols, kMaxCols].
     *
     * @throws std::invalid_argument se as dimensões estiverem fora dos limites.
     */
    explicit Board(int rows, int cols);

    /** @brief Destrutor padrão. As unique_ptr limpam as entidades. */
    ~Board() = default;

    // Board é um objeto com identidade única — cópia desabilitada.
    Board(const Board&)            = delete;
    Board& operator=(const Board&) = delete;

    // Move permitido para transferência de posse (ex.: LevelLoader → BoardEngine).
    Board(Board&&)                 = default;
    Board& operator=(Board&&)      = default;

    // ─── Dimensões ─────────────────────────────────────────────────────────

    /**
     * @brief Devolve o número de linhas do tabuleiro.
     */
    [[nodiscard]] int rows() const noexcept;

    /**
     * @brief Devolve o número de colunas do tabuleiro.
     */
    [[nodiscard]] int cols() const noexcept;

    /**
     * @brief Indica se a posição dada está dentro dos limites do tabuleiro.
     *
     * @param pos Posição a verificar (col=x, row=y).
     * @return true se 0 ≤ pos.x() < cols_ && 0 ≤ pos.y() < rows_.
     */
    [[nodiscard]] bool isValidPosition(QPoint pos) const noexcept;

    /**
     * @brief Sobrecarga por coordenadas explícitas.
     */
    [[nodiscard]] bool isValidPosition(int row, int col) const noexcept;

    // ─── Adição de entidades ───────────────────────────────────────────────

    /**
     * @brief Adiciona um autocarro ao tabuleiro.
     *
     * Toma posse do objeto. Atualiza a grelha de ocupação.
     *
     * Pré-condições validadas:
     *  - bus != nullptr
     *  - A posição inicial do Bus está dentro dos limites.
     *  - A célula de destino está livre (OccupancyType::None).
     *  - Não existe já um Bus com o mesmo id.
     *
     * @param bus Ponteiro único para o autocarro a adicionar.
     * @return true se adicionado com sucesso; false caso contrário.
     */
    bool addBus(std::unique_ptr<Bus> bus);

    /**
     * @brief Adiciona um passageiro ao tabuleiro.
     *
     * Toma posse do objeto. Atualiza a grelha de ocupação.
     *
     * Pré-condições validadas:
     *  - passenger != nullptr
     *  - A posição está dentro dos limites.
     *  - A célula está livre.
     *  - Não existe já um Passenger com o mesmo id.
     *
     * @param passenger Ponteiro único para o passageiro a adicionar.
     * @return true se adicionado com sucesso; false caso contrário.
     */
    bool addPassenger(std::unique_ptr<Passenger> passenger);

    /**
     * @brief Adiciona uma plataforma ao tabuleiro.
     *
     * Toma posse do objeto. Atualiza a grelha de ocupação.
     * O número de plataformas é validado contra [kMinPlatforms, kMaxPlatforms]
     * apenas quando commitPlatforms() é chamado.
     *
     * Pré-condições validadas:
     *  - platform != nullptr
     *  - A posição está dentro dos limites.
     *  - A célula está livre.
     *  - Não existe já uma Platform com o mesmo id.
     *
     * @param platform Ponteiro único para a plataforma a adicionar.
     * @return true se adicionada com sucesso; false caso contrário.
     */
    bool addPlatform(std::unique_ptr<Platform> platform);

    /**
     * @brief Valida que o número de plataformas está dentro dos limites do jogo.
     *
     * Deve ser chamado pelo LevelLoader após adicionar todas as plataformas,
     * antes de passar o Board ao BoardEngine.
     *
     * @return true se kMinPlatforms ≤ platformCount() ≤ kMaxPlatforms.
     */
    [[nodiscard]] bool validatePlatformCount() const;

    // ─── Contagem de entidades ─────────────────────────────────────────────

    /** @brief Devolve o número de autocarros (incluindo os que já saíram). */
    [[nodiscard]] int busCount() const noexcept;

    /** @brief Devolve o número de passageiros. */
    [[nodiscard]] int passengerCount() const noexcept;

    /** @brief Devolve o número de plataformas. */
    [[nodiscard]] int platformCount() const noexcept;

    // ─── Acesso a entidades por id ─────────────────────────────────────────

    /**
     * @brief Devolve um ponteiro observador para o Bus com o id dado.
     *
     * @param id Id do autocarro.
     * @return Ponteiro para Bus, ou nullptr se não existir.
     *
     * @note O ponteiro é válido enquanto o Board existir e o Bus não for
     *       removido. Não guardar além do âmbito imediato.
     */
    [[nodiscard]] Bus* findBusById(int id);

    /** @brief Versão const. */
    [[nodiscard]] const Bus* findBusById(int id) const;

    /**
     * @brief Devolve um ponteiro observador para o Passenger com o id dado.
     */
    [[nodiscard]] Passenger* findPassengerById(int id);

    /** @brief Versão const. */
    [[nodiscard]] const Passenger* findPassengerById(int id) const;

    /**
     * @brief Devolve um ponteiro observador para a Platform com o id dado.
     */
    [[nodiscard]] Platform* findPlatformById(int id);

    /** @brief Versão const. */
    [[nodiscard]] const Platform* findPlatformById(int id) const;

    // ─── Acesso por posição ────────────────────────────────────────────────

    /**
     * @brief Devolve informação sobre o que ocupa a célula em pos.
     *
     * Consulta O(1) na occupancyGrid_.
     *
     * @param pos Posição na grelha.
     * @return CellInfo com tipo e id da entidade, ou {None, -1} se vazia.
     *         Devolve {None, -1} também se pos for inválida.
     */
    [[nodiscard]] CellInfo cellInfoAt(QPoint pos) const noexcept;

    /** @brief Sobrecarga por coordenadas explícitas. */
    [[nodiscard]] CellInfo cellInfoAt(int row, int col) const noexcept;

    /**
     * @brief Indica se a célula em pos está livre (OccupancyType::None).
     *
     * @param pos Posição a verificar.
     * @return true se livre ou se pos é inválida (fora dos limites).
     *
     * @note "Livre" não significa que pode ser ocupada — pode estar fora
     *       dos limites. Use isValidPosition() + isCellFree() em conjunto
     *       para verificar se uma célula pode ser ocupada.
     */
    [[nodiscard]] bool isCellFree(QPoint pos) const noexcept;

    /** @brief Sobrecarga por coordenadas explícitas. */
    [[nodiscard]] bool isCellFree(int row, int col) const noexcept;

    /**
     * @brief Devolve o Bus que ocupa a posição dada, ou nullptr.
     *
     * Combinação de cellInfoAt() + findBusById() para conveniência.
     */
    [[nodiscard]] Bus* busAt(QPoint pos);

    /** @brief Versão const. */
    [[nodiscard]] const Bus* busAt(QPoint pos) const;

    /**
     * @brief Devolve o Passenger que ocupa a posição dada, ou nullptr.
     */
    [[nodiscard]] Passenger* passengerAt(QPoint pos);

    /** @brief Versão const. */
    [[nodiscard]] const Passenger* passengerAt(QPoint pos) const;

    /**
     * @brief Devolve a Platform que ocupa a posição dada, ou nullptr.
     */
    [[nodiscard]] Platform* platformAt(QPoint pos);

    /** @brief Versão const. */
    [[nodiscard]] const Platform* platformAt(QPoint pos) const;

    // ─── Acesso às coleções completas (leitura) ────────────────────────────

    /**
     * @brief Devolve referência const à lista de autocarros.
     *
     * Usado pelo BoardEngine para iterar sobre todos os autocarros
     * (ex.: verificar condição de vitória, render list para QML).
     */
    [[nodiscard]] const std::vector<std::unique_ptr<Bus>>& buses() const noexcept;

    /**
     * @brief Devolve referência const à lista de passageiros.
     */
    [[nodiscard]] const std::vector<std::unique_ptr<Passenger>>& passengers() const noexcept;

    /**
     * @brief Devolve referência const à lista de plataformas.
     */
    [[nodiscard]] const std::vector<std::unique_ptr<Platform>>& platforms() const noexcept;

    // ─── Gestão da grelha de ocupação ─────────────────────────────────────

    /**
     * @brief Regista a ocupação de uma célula por uma entidade.
     *
     * Chamado internamente (addBus, addPassenger, addPlatform) e pelo
     * BoardEngine quando um Bus se move.
     *
     * @param pos  Posição da célula.
     * @param type Tipo de entidade.
     * @param id   Id da entidade.
     * @return true se a operação foi bem-sucedida; false se pos inválida
     *         ou célula já ocupada.
     */
    bool occupyCell(QPoint pos, OccupancyType type, int id);

    /**
     * @brief Liberta uma célula, marcando-a como None.
     *
     * Chamado pelo BoardEngine quando um Bus avança para a célula seguinte
     * ou quando um Passenger embarca.
     *
     * @param pos Posição a libertar.
     * @return true se bem-sucedido; false se pos inválida ou já livre.
     */
    bool freeCell(QPoint pos);

    /**
     * @brief Executa a atualização atômica da posição de um Bus na grelha.
     *
     * Liberta a célula anterior e ocupa a nova numa única operação validada.
     * Usado pelo BoardEngine após Bus::mover().
     *
     * @param busId   Id do autocarro a mover.
     * @param fromPos Posição anterior (deve estar marcada com este Bus).
     * @param toPos   Posição nova (deve estar livre e dentro dos limites).
     * @return true se a atualização foi bem-sucedida; false caso contrário.
     */
    bool moveBusOnGrid(int busId, QPoint fromPos, QPoint toPos);

    // ─── Regras de movimento ──────────────────────────────────────────────

    /**
     * @brief Verifica se um autocarro pode avançar um passo na sua direção.
     *
     * Condições:
     *  - O Bus existe e está ativo.
     *  - nextPosition() está dentro dos limites do tabuleiro.
     *  - A célula seguinte está livre OU é uma plataforma da mesma cor.
     *
     * @param busId Id do autocarro.
     * @return true se o avanço é possível.
     */
    [[nodiscard]] bool canBusAdvance(int busId) const;

    /**
     * @brief Verifica se o caminho de um autocarro até à sua plataforma
     *        está completamente livre de obstáculos.
     *
     * Percorre todas as células entre a posição atual do Bus e a plataforma
     * destino (exclusive), verificando que estão todas livres ou são a
     * plataforma de destino.
     *
     * @param busId Id do autocarro.
     * @return true se o caminho está livre; false se há obstáculo ou o Bus
     *         não existe.
     */
    [[nodiscard]] bool isPathToplatformClear(int busId) const;

    /**
     * @brief Encontra a plataforma de destino para um dado autocarro.
     *
     * A plataforma correta é a que tem a mesma cor que o Bus e está na
     * direção do seu movimento.
     *
     * @param busId Id do autocarro.
     * @return Ponteiro const para a Platform destino, ou nullptr se não
     *         existir plataforma compatível.
     */
    [[nodiscard]] const Platform* findTargetPlatform(int busId) const;

    /** @brief Versão não-const. */
    [[nodiscard]] Platform* findTargetPlatform(int busId);

    // ─── Condição de vitória ──────────────────────────────────────────────

    /**
     * @brief Verifica se todos os passageiros embarcaram num autocarro.
     *
     * @return true se todos os Passenger têm state == Boarded.
     */
    [[nodiscard]] bool allPassengersBoarded() const;

    /**
     * @brief Verifica se todos os autocarros saíram do tabuleiro.
     *
     * @return true se todos os Bus têm hasLeft() == true.
     */
    [[nodiscard]] bool allBusesLeft() const;

    /**
     * @brief Verifica se o nível foi concluído com sucesso.
     *
     * Combina allPassengersBoarded() && allBusesLeft().
     * O BoardEngine deve chamar este método após cada ação relevante.
     *
     * @return true se o nível está completo.
     */
    [[nodiscard]] bool isLevelComplete() const;

    /**
     * @brief Devolve o número de passageiros que ainda aguardam embarque.
     */
    [[nodiscard]] int waitingPassengerCount() const;

    /**
     * @brief Devolve o número de autocarros ainda ativos no tabuleiro.
     */
    [[nodiscard]] int activeBusCount() const;

    // ─── Reset ─────────────────────────────────────────────────────────────

    /**
     * @brief Restaura o tabuleiro completo ao estado inicial do nível.
     *
     * Operações executadas:
     *  1. Limpa toda a occupancyGrid_.
     *  2. Chama reset() em cada Bus, Passenger e Platform.
     *  3. Reconstrói a occupancyGrid_ com as posições iniciais.
     *
     * Não destrói nem recria entidades — os ids e atributos const
     * permanecem inalterados. Seguro para chamar múltiplas vezes.
     */
    void reset();

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual do tabuleiro para logs e depuração.
     *
     * Inclui dimensões, contagem de entidades e estado da grelha.
     * Formato:
     *   "Board[8x8 buses=4 passengers=12 platforms=4 complete=false]"
     */
    [[nodiscard]] QString toString() const;

    /**
     * @brief Devolve uma representação ASCII da grelha para depuração.
     *
     * Legenda:
     *  '.' = vazio, 'B' = Bus, 'P' = Passenger, 'X' = Platform
     *
     * @return String multi-linha com a grelha completa.
     */
    [[nodiscard]] QString toAsciiGrid() const;

private:
    // ─── Dimensões ─────────────────────────────────────────────────────────

    const int rows_; ///< Número de linhas do tabuleiro.
    const int cols_; ///< Número de colunas do tabuleiro.

    // ─── Entidades (posse exclusiva via unique_ptr) ────────────────────────

    std::vector<std::unique_ptr<Bus>>       buses_;
    std::vector<std::unique_ptr<Passenger>> passengers_;
    std::vector<std::unique_ptr<Platform>>  platforms_;

    // ─── Grelha de ocupação ────────────────────────────────────────────────

    /**
     * @brief Grelha de ocupação rows_ × cols_.
     *
     * Layout: occupancyGrid_[row][col].
     * Acesso O(1) para consultas de colisão.
     * Armazenada como QVector<QVector<>> para suportar dimensões dinâmicas.
     */
    QVector<QVector<OccupancyType>> occupancyGrid_;

    /**
     * @brief Mapa auxiliar de posição → id da entidade.
     *
     * Paralelo à occupancyGrid_, guarda o id da entidade que ocupa cada
     * célula. Permite que cellInfoAt() devolva id + tipo em O(1).
     * Células vazias têm valor -1.
     */
    QVector<QVector<int>> idGrid_;

    // ─── Auxiliares privados ──────────────────────────────────────────────

    /**
     * @brief Inicializa ambas as grelhas com valores sentinela.
     *
     * Chamado no construtor e no reset().
     */
    void clearGrids();

    /**
     * @brief Reconstrói as grelhas a partir das posições atuais das entidades.
     *
     * Chamado no final de reset() após chamar reset() em cada entidade.
     */
    void rebuildGrids();

    /**
     * @brief Verifica se a célula na posição dada pode ser ocupada.
     *
     * Combina isValidPosition() e isCellFree() numa única verificação
     * com log unificado.
     *
     * @param pos    Posição a verificar.
     * @param caller Nome do método chamador (para mensagens de aviso).
     * @return true se a posição é válida e a célula está livre.
     */
    [[nodiscard]] bool canOccupyCell(QPoint pos, const char* caller) const;

    /**
     * @brief Verifica se um id de entidade é duplicado numa lista.
     *
     * Template para reutilização em addBus, addPassenger, addPlatform.
     *
     * @tparam T    Tipo da entidade (Bus, Passenger, Platform).
     * @param list  Lista a pesquisar.
     * @param id    Id a verificar.
     * @return true se o id já existe na lista.
     */
    template<typename T>
    [[nodiscard]] bool isDuplicateId(const std::vector<std::unique_ptr<T>>& list,
                                     int id) const;

    /**
     * @brief Pesquisa linear por id numa lista de unique_ptr de entidades.
     *
     * @tparam T Tipo da entidade.
     * @param list Lista a pesquisar.
     * @param id   Id a encontrar.
     * @return Ponteiro para a entidade, ou nullptr se não existir.
     */
    template<typename T>
    [[nodiscard]] static T* findById(const std::vector<std::unique_ptr<T>>& list,
                                     int id);

    /**
     * @brief Devolve o carácter ASCII para um tipo de ocupação.
     * Usado por toAsciiGrid().
     */
    [[nodiscard]] static char occupancyChar(OccupancyType type) noexcept;
};
