/**
 * @file BoardEngine.h
 * @brief Declaração da classe BoardEngine para o jogo Bus Jam Puzzle.
 *
 * BoardEngine é o orquestrador da lógica de jogo em tempo real.
 * Opera sobre um Board existente (passado por referência) e executa
 * todas as mutações de estado que decorrem de ações do jogador ou
 * do ciclo de jogo.
 *
 * ### Divisão de responsabilidades:
 *
 *  Board       → estrutura, repositório, consultas, invariantes posicionais.
 *  BoardEngine → lógica de jogo, sequência de ações, decisões, histórico.
 *
 *  BoardEngine nunca acede diretamente às grelhas internas de Board.
 *  Usa apenas a API pública de Board, Bus, Passenger e Platform.
 *
 * ### Fluxo de uma jogada típica:
 *
 *  1. GameController chama requestBusMove(busId).
 *  2. BoardEngine valida o pedido (bus existe, está ativo, caminho livre).
 *  3. Se o caminho até à plataforma está livre → executaFullMove():
 *       a. Guarda snapshot no histórico (Undo).
 *       b. Move o Bus célula a célula até à plataforma.
 *       c. Chama Platform::receberBus().
 *       d. Embarca passageiros compatíveis (Boarding → Boarded).
 *       e. Chama Bus::sairDoTabuleiro() e Platform::concluirPlatform().
 *       f. Verifica condição de vitória.
 *  4. Se o caminho está bloqueado → retorna MoveResult::Blocked.
 *
 * ### Histórico de estados (Undo):
 *  O histórico é implementado como uma pilha de BoardSnapshot.
 *  Cada snapshot guarda o estado mínimo necessário para reconstruir o
 *  tabuleiro: posições e estados de Bus, Passenger e Platform.
 *  O Undo completo (reversão de snapshots) será implementado numa
 *  iteração futura — a estrutura está presente e documentada.
 *
 * ### Thread-safety:
 *  Não é thread-safe. Todas as chamadas devem ocorrer na thread principal.
 *  Futura paralelização: o MovementSolver (pathfinding) pode correr numa
 *  thread separada se receber cópias dos dados de entrada.
 */

#pragma once

#include "Board.h"

#include <QString>
#include <QList>
#include <QPoint>
#include <functional>   // std::function (callbacks de animação)
#include <optional>

// ─────────────────────────────────────────────────────────────────────────────
// Tipos de resultado e enumerações
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Resultado de um pedido de movimento de autocarro.
 *
 * Permite ao GameController distinguir entre os vários desfechos possíveis
 * de requestBusMove() e tomar decisões de UI adequadas.
 */
enum class MoveResult : quint8
{
    Success       = 0,  ///< Autocarro moveu-se e completou o ciclo (embarque + saída).
    Blocked       = 1,  ///< Caminho bloqueado — nenhum movimento executado.
    AlreadyLeft   = 2,  ///< Autocarro já saiu do tabuleiro.
    BusNotFound   = 3,  ///< Id de autocarro inválido.
    NoPlatform    = 4,  ///< Não existe plataforma compatível no alinhamento do Bus.
    InvalidState  = 5   ///< BoardEngine em estado que não permite movimentos.
};

/**
 * @brief Estado geral do motor de jogo.
 */
enum class EngineState : quint8
{
    Ready      = 0,  ///< Pronto para receber pedidos de movimento.
    Processing = 1,  ///< A processar uma jogada (evita reentrada).
    Won        = 2,  ///< Nível concluído com sucesso.
    Idle       = 3   ///< Sem Board carregado; chamadas de movimento são recusadas.
};

// ─────────────────────────────────────────────────────────────────────────────
// Estrutura de snapshot para Undo
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Estado mínimo de um Bus num momento específico.
 */
struct BusSnapshot
{
    int       id;
    QPoint    position;
    int       boardedCount;
    bool      isActive;
    bool      hasLeft;
    bool      isAtPlatform;
};

/**
 * @brief Estado mínimo de um Passenger num momento específico.
 */
struct PassengerSnapshot
{
    int            id;
    PassengerState state;
    int            busId;
};

/**
 * @brief Estado mínimo de uma Platform num momento específico.
 */
struct PlatformSnapshot
{
    int           id;
    PlatformState state;
    int           currentBusId;
};

/**
 * @brief Snapshot completo do tabuleiro para suporte a Undo.
 *
 * Guarda apenas o estado mutável das entidades. Os atributos const
 * (ids, cores, capacidades, posições iniciais) não são incluídos —
 * são recuperados das entidades originais.
 *
 * O BoardEngine guarda uma pilha destas estruturas.
 * A reversão (Undo real) será implementada no BoardEngine::undoLastMove().
 */
struct BoardSnapshot
{
    QList<BusSnapshot>       buses;
    QList<PassengerSnapshot> passengers;
    QList<PlatformSnapshot>  platforms;
    int                      moveNumber { 0 };  ///< Número da jogada guardada.
};

// ─────────────────────────────────────────────────────────────────────────────
// Constantes do módulo
// ─────────────────────────────────────────────────────────────────────────────

namespace BoardEngineConstants
{
    /// Número máximo de snapshots mantidos na pilha de Undo.
    constexpr int kMaxUndoHistory = 50;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tipo de callback para notificação de eventos
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Callback invocado após cada evento de jogo significativo.
 *
 * Permite ao GameController (e futuramente ao QML) reagir a eventos
 * sem que o BoardEngine conheça a camada de apresentação.
 *
 * Exemplos de uso:
 *   engine.setOnBusMoved([](int busId, QPoint from, QPoint to){ ... });
 *   engine.setOnLevelComplete([]{ ui->showVictoryScreen(); });
 */
using BusMovedCallback      = std::function<void(int busId, QPoint from, QPoint to)>;
using PassengerBoardedCallback = std::function<void(int passengerId, int busId)>;
using LevelCompleteCallback = std::function<void()>;
using BusLeftCallback       = std::function<void(int busId)>;

// ─────────────────────────────────────────────────────────────────────────────
// Classe BoardEngine
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Motor de lógica de jogo para o Bus Jam Puzzle.
 *
 * Recebe um Board já construído e com entidades adicionadas, e opera
 * sobre ele durante o ciclo de vida de um nível.
 *
 * ### Posse de memória:
 *  BoardEngine não possui o Board — recebe uma referência. O Board deve
 *  ter tempo de vida >= ao BoardEngine. O GameController é responsável
 *  pela gestão de lifetime de ambos.
 */
class BoardEngine
{
public:
    // ─── Construção e destruição ───────────────────────────────────────────

    /**
     * @brief Constrói o engine associado a um Board existente.
     *
     * @param board Referência ao Board que este engine vai operar.
     *              O Board deve existir enquanto o BoardEngine existir.
     *
     * O engine inicia em estado Ready se o Board tiver pelo menos
     * um Bus e uma Platform; Idle caso contrário.
     */
    explicit BoardEngine(Board& board);

    /** @brief Destrutor padrão. */
    ~BoardEngine() = default;

    // BoardEngine tem identidade única — cópia desabilitada.
    BoardEngine(const BoardEngine&)            = delete;
    BoardEngine& operator=(const BoardEngine&) = delete;

    // Move desabilitado: a referência a Board tornaria o move ambíguo.
    BoardEngine(BoardEngine&&)                 = delete;
    BoardEngine& operator=(BoardEngine&&)      = delete;

    // ─── Estado do engine ─────────────────────────────────────────────────

    /**
     * @brief Devolve o estado atual do engine.
     */
    [[nodiscard]] EngineState state() const noexcept;

    /**
     * @brief Representação legível do estado para logs.
     */
    [[nodiscard]] QString stateName() const;

    /**
     * @brief Indica se o engine está pronto para receber movimentos.
     */
    [[nodiscard]] bool isReady() const noexcept;

    /**
     * @brief Indica se o nível foi concluído.
     */
    [[nodiscard]] bool isWon() const noexcept;

    /**
     * @brief Devolve o número de jogadas executadas desde o início/restart.
     */
    [[nodiscard]] int moveCount() const noexcept;

    /**
     * @brief Devolve referência const ao Board gerido por este engine.
     */
    [[nodiscard]] const Board& board() const noexcept;

    // ─── Ação principal: pedido de movimento ──────────────────────────────

    /**
     * @brief Processa um pedido de movimento para o autocarro com o id dado.
     *
     * Esta é a única entrada de input de jogador no engine. O GameController
     * chama este método quando o jogador toca/clica num autocarro.
     *
     * ### Sequência interna:
     *  1. Valida estado do engine e existência do Bus.
     *  2. Verifica se o caminho até à plataforma está livre.
     *  3. Guarda snapshot para Undo.
     *  4. Executa movimento completo: célula a célula → plataforma → embarque → saída.
     *  5. Verifica condição de vitória.
     *  6. Invoca callbacks registados.
     *
     * @param busId Id do autocarro a mover.
     * @return MoveResult indicando o resultado da operação.
     */
    MoveResult requestBusMove(int busId);

    // ─── Undo ─────────────────────────────────────────────────────────────

    /**
     * @brief Indica se existe pelo menos um estado anterior disponível.
     */
    [[nodiscard]] bool canUndo() const noexcept;

    /**
     * @brief Devolve o número de estados guardados na pilha de Undo.
     */
    [[nodiscard]] int undoStackSize() const noexcept;

    /**
     * @brief Estrutura de suporte a Undo — reversão não implementada ainda.
     *
     * A pilha de snapshots está preparada. Este método está declarado
     * para permitir integração futura sem alterar a interface pública.
     *
     * @return false sempre (não implementado).
     * @note Implementação completa prevista numa iteração futura.
     */
    bool undoLastMove();

    // ─── Restart ──────────────────────────────────────────────────────────

    /**
     * @brief Reinicia o nível ao estado inicial.
     *
     * Operações:
     *  1. Chama Board::reset() (repõe todas as entidades).
     *  2. Limpa a pilha de Undo.
     *  3. Repõe moveCount_ para 0.
     *  4. Repõe state_ para Ready.
     */
    void restart();

    // ─── Callbacks de eventos ─────────────────────────────────────────────

    /**
     * @brief Regista callback invocado após cada passo de movimento de Bus.
     *
     * @param cb Função chamada com (busId, posAnterior, posNova).
     *           Pode ser nullptr para remover o callback.
     */
    void setOnBusMoved(BusMovedCallback cb);

    /**
     * @brief Regista callback invocado quando um passageiro embarca.
     *
     * @param cb Função chamada com (passengerId, busId).
     */
    void setOnPassengerBoarded(PassengerBoardedCallback cb);

    /**
     * @brief Regista callback invocado quando um autocarro sai do tabuleiro.
     *
     * @param cb Função chamada com (busId).
     */
    void setOnBusLeft(BusLeftCallback cb);

    /**
     * @brief Regista callback invocado quando o nível é concluído.
     *
     * @param cb Função sem argumentos.
     */
    void setOnLevelComplete(LevelCompleteCallback cb);

    // ─── Consultas de estado do jogo ──────────────────────────────────────

    /**
     * @brief Devolve quantos passageiros ainda aguardam embarque.
     */
    [[nodiscard]] int remainingPassengers() const;

    /**
     * @brief Devolve quantos autocarros ainda estão ativos no tabuleiro.
     */
    [[nodiscard]] int remainingBuses() const;

    /**
     * @brief Verifica se um Bus específico pode mover-se agora.
     *
     * Usa Board::isPathToplatformClear() internamente.
     * Usado pelo GameController para feedback visual (highlighting).
     *
     * @param busId Id do autocarro.
     * @return true se o movimento pode ser executado neste momento.
     */
    [[nodiscard]] bool canBusMove(int busId) const;

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual do engine para logs e depuração.
     *
     * Formato:
     * "BoardEngine[state=Ready moves=3 undoStack=3 won=false]"
     */
    [[nodiscard]] QString toString() const;

private:
    // ─── Referência ao Board ───────────────────────────────────────────────

    Board& board_;  ///< Tabuleiro sobre o qual o engine opera. Não possui.

    // ─── Estado interno ────────────────────────────────────────────────────

    EngineState state_;      ///< Estado atual do engine.
    int         moveCount_;  ///< Jogadas executadas desde o início/restart.

    // ─── Pilha de Undo ────────────────────────────────────────────────────

    QList<BoardSnapshot> undoStack_;  ///< Histórico de snapshots (LIFO).

    // ─── Callbacks ────────────────────────────────────────────────────────

    BusMovedCallback         onBusMoved_;
    PassengerBoardedCallback onPassengerBoarded_;
    BusLeftCallback          onBusLeft_;
    LevelCompleteCallback    onLevelComplete_;

    // ─── Lógica de movimento (privada) ────────────────────────────────────

    /**
     * @brief Executa o movimento completo de um autocarro desde a sua posição
     *        atual até à plataforma, incluindo embarque e saída.
     *
     * Chamado internamente por requestBusMove() após validação.
     *
     * Sequência:
     *  1. Avança o Bus célula a célula até à posição da plataforma.
     *  2. Chama Platform::receberBus().
     *  3. Embarca todos os passageiros compatíveis via boardPassengers().
     *  4. Chama Bus::sairDoTabuleiro() e Platform::concluirPlatform().
     *
     * @param busId    Id do autocarro.
     * @param platform Ponteiro para a plataforma destino (já validado).
     * @return true se o ciclo completo foi executado com sucesso.
     */
    bool executeFullMove(int busId, Platform* platform);

    /**
     * @brief Liberta das grelhas do Board os passageiros da mesma cor do Bus
     *        que estejam no caminho entre o Bus e a plataforma destino.
     *
     * Os passageiros permanecem em estado Waiting — boardPassengers() tratará
     * das transições de estado. Esta limpeza prévia é necessária para que
     * advanceBusToPosition() possa mover o Bus sem encontrar obstáculos.
     *
     * @param busId    Id do autocarro a mover.
     * @param platform Plataforma destino (já validada).
     */
    void freePassengersOnPath(int busId, const Platform* platform);

    /**
     * @brief Avança um Bus uma célula de cada vez até atingir a posição destino.
     *
     * Atualiza a grelha do Board em cada passo via Board::moveBusOnGrid().
     * Invoca o callback onBusMoved_ após cada passo.
     *
     * @param busId  Id do autocarro.
     * @param target Posição destino (célula da plataforma).
     * @return true se o Bus atingiu a posição destino; false se encontrou
     *         obstáculo inesperado durante o percurso.
     */
    bool advanceBusToPosition(int busId, QPoint target);

    /**
     * @brief Embarca todos os passageiros compatíveis no autocarro dado.
     *
     * Itera sobre todos os Passenger em estado Waiting, verifica
     * compatibilidade de cor e invoca o ciclo
     * Passenger::iniciarEmbarque() → Bus::embarcarPassageiro() → Passenger::concluirEmbarque().
     * Liberta a célula do passageiro na grelha após embarque.
     * Invoca onPassengerBoarded_ para cada embarque bem-sucedido.
     *
     * @param bus      Ponteiro para o autocarro (já validado, na plataforma).
     * @param platform Ponteiro para a plataforma (já em estado Occupied).
     * @return Número de passageiros efetivamente embarcados.
     */
    int boardPassengers(Bus* bus, Platform* platform);

    /**
     * @brief Verifica e processa a condição de vitória.
     *
     * Chama Board::isLevelComplete(). Se verdadeiro:
     *  - Muda state_ para Won.
     *  - Invoca onLevelComplete_.
     *
     * @return true se o nível foi concluído nesta chamada.
     */
    bool checkAndHandleVictory();

    // ─── Gestão do histórico de Undo ──────────────────────────────────────

    /**
     * @brief Captura o estado atual do Board num BoardSnapshot.
     *
     * Itera sobre todas as entidades e guarda os seus atributos mutáveis.
     *
     * @return Snapshot do estado atual.
     */
    [[nodiscard]] BoardSnapshot captureSnapshot() const;

    /**
     * @brief Empurra um snapshot para o topo da pilha de Undo.
     *
     * Se a pilha atingir kMaxUndoHistory, o snapshot mais antigo é removido.
     *
     * @param snapshot Snapshot a guardar.
     */
    void pushSnapshot(BoardSnapshot snapshot);

    /**
     * @brief Aplica um snapshot ao Board, restaurando o estado das entidades.
     *
     * Usado por undoLastMove(). Ainda não implementado na versão atual.
     *
     * @param snapshot Snapshot a restaurar.
     */
    void applySnapshot(const BoardSnapshot& snapshot);

    // ─── Auxiliares de validação ──────────────────────────────────────────

    /**
     * @brief Valida as pré-condições comuns de requestBusMove().
     *
     * Verifica: estado do engine, existência do Bus, Bus ativo, não saiu.
     *
     * @param busId     Id do autocarro.
     * @param outBus    [out] Ponteiro para o Bus encontrado (se bem-sucedido).
     * @param outResult [out] MoveResult de falha (se mal-sucedido).
     * @return true se todas as pré-condições passaram.
     */
    [[nodiscard]] bool validateMoveRequest(int        busId,
                                           Bus**      outBus,
                                           MoveResult& outResult) const;

    /**
     * @brief Devolve o nome textual de um MoveResult para logs.
     */
    [[nodiscard]] static const char* moveResultName(MoveResult result) noexcept;

    /**
     * @brief Devolve o nome textual de um EngineState para logs.
     */
    [[nodiscard]] static const char* engineStateName(EngineState state) noexcept;
};
