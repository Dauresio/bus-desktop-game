/**
 * @file Bus.h
 * @brief Declaração da classe Bus para o jogo Bus Jam Puzzle.
 *
 * A classe Bus representa um autocarro no tabuleiro do jogo.
 * É uma entidade de domínio pura: não depende de QML nem de qualquer
 * subsistema de apresentação. Toda a lógica de regras de negócio
 * relativa ao autocarro reside aqui.
 *
 * Capacidades válidas: 4, 6, 8 e 12 passageiros.
 * O autocarro só aceita passageiros da mesma cor.
 * O movimento é sempre para a frente (conforme a direção definida).
 * Após sair do tabuleiro, o autocarro fica inativo e não pode mover.
 */

#pragma once

#include <QtGlobal>   // qint32, Q_DISABLE_COPY
#include <QString>
#include <QPoint>
#include <QSet>

// ─────────────────────────────────────────────────────────────────────────────
// Enumerações partilhadas (declaradas aqui; movidas para Enums.h quando
// as restantes classes forem implementadas).
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Cores disponíveis para autocarros e passageiros.
 *
 * Cada cor identifica univocamente um grupo de passageiros e a plataforma
 * correspondente. Um passageiro só embarca num autocarro da mesma cor.
 */
enum class BusColor : quint8
{
    Red    = 0,
    Blue   = 1,
    Green  = 2,
    Yellow = 3,
    Purple = 4
};

/**
 * @brief Direção fixa de deslocamento do autocarro no tabuleiro.
 *
 * A direção é definida na criação do autocarro e nunca muda.
 * O autocarro move-se sempre nesta direção até atingir a plataforma.
 */
enum class Direction : quint8
{
    Up    = 0,
    Down  = 1,
    Left  = 2,
    Right = 3
};

// ─────────────────────────────────────────────────────────────────────────────
// Conjunto de capacidades válidas (constante global deste módulo)
// ─────────────────────────────────────────────────────────────────────────────

namespace BusConstants
{
    /// Valores de capacidade aceites pelo construtor e pelo setter.
    constexpr std::initializer_list<int> ValidCapacities = { 4, 6, 8, 12 };

    /// Posição sentinela usada quando o autocarro está fora do tabuleiro.
    constexpr QPoint InvalidPosition { -1, -1 };
}

// ─────────────────────────────────────────────────────────────────────────────
// Classe Bus
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Entidade de domínio que representa um autocarro no Bus Jam Puzzle.
 *
 * ### Ciclo de vida típico num nível:
 *  1. Construção com id, cor, capacidade, posição inicial e direção.
 *  2. O BoardEngine chama tryMove() / mover() para avançar células.
 *  3. Quando o caminho está livre, o autocarro move-se para a plataforma.
 *  4. embarcarPassageiro() é chamado para cada passageiro compatível.
 *  5. sairDoTabuleiro() marca o autocarro como inativo e remove-o da grelha.
 *  6. reset() restaura o estado inicial para o botão "Recomeçar".
 *
 * ### Invariantes da classe:
 *  - capacity_ ∈ { 4, 6, 8, 12 }
 *  - boardedCount_ ≤ capacity_
 *  - Se hasLeft_ == true → isActive_ == false
 *  - Se hasLeft_ == true → isAtPlatform_ == false
 */
class Bus
{
public:
    // ─── Construção e destruição ───────────────────────────────────────────

    /**
     * @brief Constrói um autocarro com todos os atributos necessários.
     *
     * @param id         Identificador único no contexto do nível.
     * @param color      Cor do autocarro (deve corresponder a uma plataforma).
     * @param capacity   Número máximo de passageiros. Deve ser 4, 6, 8 ou 12.
     * @param startPos   Posição inicial na grelha (linha, coluna).
     * @param direction  Direção fixa de movimento.
     *
     * @throws std::invalid_argument se capacity não for um valor válido.
     */
    explicit Bus(int        id,
                 BusColor   color,
                 int        capacity,
                 QPoint     startPos,
                 Direction  direction);

    /**
     * @brief Destrutor padrão.
     */
    ~Bus() = default;

    // Autocarros são entidades únicas — cópia desabilitada.
    // Mover é permitido (ex.: inserção em containers Qt).
    Bus(const Bus&)            = delete;
    Bus& operator=(const Bus&) = delete;
    Bus(Bus&&)                 = default;
    Bus& operator=(Bus&&)      = default;

    // ─── Identificação ─────────────────────────────────────────────────────

    /**
     * @brief Devolve o identificador único do autocarro neste nível.
     */
    [[nodiscard]] int id() const noexcept;

    // ─── Cor ───────────────────────────────────────────────────────────────

    /**
     * @brief Devolve a cor do autocarro.
     */
    [[nodiscard]] BusColor color() const noexcept;

    /**
     * @brief Representação legível da cor (útil para logs e depuração).
     * @return Ex.: "Red", "Blue", …
     */
    [[nodiscard]] QString colorName() const;

    // ─── Capacidade ────────────────────────────────────────────────────────

    /**
     * @brief Devolve a capacidade máxima de passageiros.
     * @return Valor em { 4, 6, 8, 12 }.
     */
    [[nodiscard]] int capacity() const noexcept;

    /**
     * @brief Devolve quantos passageiros estão atualmente embarcados.
     */
    [[nodiscard]] int boardedCount() const noexcept;

    /**
     * @brief Indica se o autocarro atingiu a sua capacidade máxima.
     */
    [[nodiscard]] bool isFull() const noexcept;

    /**
     * @brief Devolve o número de lugares ainda disponíveis.
     */
    [[nodiscard]] int availableSeats() const noexcept;

    // ─── Posição ───────────────────────────────────────────────────────────

    /**
     * @brief Devolve a posição atual na grelha (linha, coluna).
     *
     * Devolve BusConstants::InvalidPosition se o autocarro já saiu
     * do tabuleiro.
     */
    [[nodiscard]] QPoint position() const noexcept;

    /**
     * @brief Devolve a linha atual (atalho para position().y()).
     */
    [[nodiscard]] int row() const noexcept;

    /**
     * @brief Devolve a coluna atual (atalho para position().x()).
     */
    [[nodiscard]] int col() const noexcept;

    /**
     * @brief Define diretamente a posição do autocarro na grelha.
     *
     * Deve ser usado apenas pelo BoardEngine durante a inicialização
     * ou resolução de movimentos. Não valida obstáculos.
     *
     * @pre O autocarro não pode ter saído do tabuleiro (hasLeft_ == false).
     */
    void setPosition(QPoint newPos);

    // ─── Direção ───────────────────────────────────────────────────────────

    /**
     * @brief Devolve a direção fixa de movimento do autocarro.
     */
    [[nodiscard]] Direction direction() const noexcept;

    /**
     * @brief Representação legível da direção.
     * @return Ex.: "Up", "Down", "Left", "Right".
     */
    [[nodiscard]] QString directionName() const;

    /**
     * @brief Calcula a próxima posição se o autocarro avançar um passo.
     *
     * Não executa o movimento — apenas calcula o destino.
     * O chamador (BoardEngine / MovementSolver) é responsável por
     * verificar se essa célula está livre.
     *
     * @return Posição seguinte na direção atual.
     */
    [[nodiscard]] QPoint nextPosition() const noexcept;

    // ─── Estado ────────────────────────────────────────────────────────────

    /**
     * @brief Indica se o autocarro está ativo no tabuleiro.
     *
     * Um autocarro ativo ainda não saiu do tabuleiro.
     * Autocarros inativos não participam na deteção de colisões.
     */
    [[nodiscard]] bool isActive() const noexcept;

    /**
     * @brief Indica se o autocarro já saiu definitivamente do tabuleiro.
     *
     * Após sair, o autocarro não pode mover-se nem embarcar passageiros.
     */
    [[nodiscard]] bool hasLeft() const noexcept;

    /**
     * @brief Indica se o autocarro está atualmente numa plataforma de embarque.
     *
     * Um autocarro numa plataforma pode embarcar passageiros compatíveis,
     * mas ainda não saiu do tabuleiro.
     */
    [[nodiscard]] bool isAtPlatform() const noexcept;

    // ─── Ações principais ─────────────────────────────────────────────────

    /**
     * @brief Avança o autocarro uma célula na sua direção.
     *
     * Este método executa o deslocamento posicional. A responsabilidade
     * de verificar se o caminho está livre é do BoardEngine / MovementSolver.
     *
     * @return true se o movimento foi executado; false se o autocarro está
     *         inativo, já saiu do tabuleiro, ou a posição seria inválida.
     *
     * @note Não valida limites do tabuleiro — isso cabe ao BoardEngine.
     */
    bool mover();

    /**
     * @brief Verifica se este autocarro pode embarcar um passageiro.
     *
     * Condições para embarque:
     *  - O autocarro está ativo e não saiu do tabuleiro.
     *  - O autocarro está numa plataforma (isAtPlatform_ == true).
     *  - O autocarro não está cheio.
     *  - A cor do passageiro coincide com a cor do autocarro.
     *
     * @param passengerColor Cor do passageiro que tenta embarcar.
     * @return true se o embarque é permitido pelas regras do jogo.
     */
    [[nodiscard]] bool podeEmbarcar(BusColor passengerColor) const noexcept;

    /**
     * @brief Embarca um passageiro, incrementando o contador interno.
     *
     * Chame sempre podeEmbarcar() antes de chamar este método.
     * Emite um aviso (qWarning) se as pré-condições não forem satisfeitas,
     * mas não lança exceção (contrato de Qt).
     *
     * @param passengerColor Cor do passageiro a embarcar.
     * @return true se o embarque foi bem-sucedido; false caso contrário.
     */
    bool embarcarPassageiro(BusColor passengerColor);

    /**
     * @brief Marca o autocarro como tendo saído do tabuleiro.
     *
     * Efeitos:
     *  - isActive_      → false
     *  - isAtPlatform_  → false
     *  - hasLeft_       → true
     *  - position_      → BusConstants::InvalidPosition
     *
     * @return true se a operação foi bem-sucedida; false se o autocarro
     *         já tinha saído anteriormente.
     */
    bool sairDoTabuleiro();

    /**
     * @brief Marca o autocarro como estando numa plataforma de embarque.
     *
     * Deve ser chamado pelo BoardEngine quando o autocarro atinge
     * a célula da plataforma correspondente.
     *
     * @param atPlatform true para ativar o estado de plataforma.
     */
    void setAtPlatform(bool atPlatform) noexcept;

    /**
     * @brief Restaura o autocarro ao estado inicial do nível.
     *
     * Utilizado pelo botão "Recomeçar". Restaura:
     *  - position_      → startPosition_
     *  - boardedCount_  → 0
     *  - isActive_      → true
     *  - hasLeft_       → false
     *  - isAtPlatform_  → false
     */
    void reset() noexcept;

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual do autocarro para logs e depuração.
     *
     * Formato: "Bus[id=1 color=Red cap=6 boarded=3 pos=(2,4) dir=Right
     *            active=true atPlatform=false hasLeft=false]"
     */
    [[nodiscard]] QString toString() const;

    /**
     * @brief Verifica se a capacidade dada é um valor permitido pelo jogo.
     *
     * @param capacity Valor a verificar.
     * @return true se capacity ∈ { 4, 6, 8, 12 }.
     */
    [[nodiscard]] static bool isValidCapacity(int capacity) noexcept;

private:
    // ─── Atributos imutáveis após construção ──────────────────────────────

    const int       id_;            ///< Identificador único no nível.
    const BusColor  color_;         ///< Cor do autocarro.
    const int       capacity_;      ///< Capacidade máxima (4, 6, 8 ou 12).
    const QPoint    startPosition_; ///< Posição de início (para reset).
    const Direction direction_;     ///< Direção fixa de movimento.

    // ─── Estado mutável ───────────────────────────────────────────────────

    QPoint  position_;      ///< Posição atual na grelha.
    int     boardedCount_;  ///< Passageiros atualmente embarcados.
    bool    isActive_;      ///< true enquanto estiver no tabuleiro.
    bool    hasLeft_;       ///< true após sairDoTabuleiro().
    bool    isAtPlatform_;  ///< true quando está sobre uma plataforma.

    // ─── Auxiliares privados ──────────────────────────────────────────────

    /**
     * @brief Calcula o delta de posição para um passo na direção dada.
     * @return QPoint com (deltaCol, deltaRow).
     */
    [[nodiscard]] static QPoint directionDelta(Direction dir) noexcept;
};
