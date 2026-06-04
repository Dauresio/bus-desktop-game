/**
 * @file MovementSolver.h
 * @brief Declaração da classe MovementSolver para o jogo Bus Jam Puzzle.
 *
 * MovementSolver encapsula todo o raciocínio de pathfinding e análise
 * de movimentos possíveis. Extrai a lógica que estava inline no BoardEngine,
 * tornando-a testável e reutilizável de forma independente.
 *
 * ### Responsabilidades:
 *  - Determinar se um Bus pode mover-se agora (caminho livre?).
 *  - Calcular o caminho completo de um Bus até à sua plataforma.
 *  - Encontrar todos os Buses que podem mover-se num dado estado.
 *  - Detectar situações de deadlock (nenhum Bus pode mover-se).
 *  - Calcular sugestões de hint (melhor Bus a mover).
 *  - Analisar dependências: qual Bus bloqueia qual.
 *
 * ### Sem estado próprio:
 *  MovementSolver é um serviço stateless. Recebe uma referência const
 *  ao Board em cada chamada e não guarda estado entre chamadas.
 *  Pode ser usado em paralelo com snapshots do Board (thread-safe
 *  desde que o Board não seja mutado durante a análise).
 *
 * ### Diferença em relação ao BoardEngine:
 *  BoardEngine    → executa movimentos (muta Board, Bus, Platform).
 *  MovementSolver → analisa possibilidades (só lê, nunca muta).
 *
 * ### Algoritmos usados:
 *
 *  - **Verificação de caminho livre** (`isPathClear`):
 *    Percorre as células entre a posição atual do Bus e a plataforma,
 *    verificando colisão com OccupancyType != None (excluindo Platform destino).
 *    O(n) onde n é a distância ao destino.
 *
 *  - **Análise de dependências** (`buildDependencyGraph`):
 *    Para cada Bus bloqueado, identifica qual entidade na célula seguinte
 *    o bloqueia. Gera uma lista de pares (bloqueado, bloqueador).
 *    O(B × D) onde B = número de buses, D = profundidade máxima de bloqueio.
 *
 *  - **Sugestão de hint** (`suggestHint`):
 *    Heurística: prefere o Bus que, ao mover-se, desbloqueia mais outros.
 *    Entre empatados, prefere o mais próximo da plataforma.
 *
 *  - **Detecção de deadlock** (`isDeadlocked`):
 *    Verifica se o conjunto de Buses móveis é vazio E se todos os Buses
 *    inativos têm pelo menos um Passenger por embarcar.
 */

#pragma once

#include "Board.h"

#include <QList>
#include <QPoint>
#include <optional>

// ─────────────────────────────────────────────────────────────────────────────
// Estruturas de resultado
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Caminho de um Bus desde a posição atual até à plataforma destino.
 */
struct BusPath
{
    int         busId    { -1 };
    QList<QPoint> steps;           ///< Posições intermédias, incluindo destino.
    int         platformId { -1 };
    bool        isValid  { false };

    /** @brief Número de passos até à plataforma. */
    [[nodiscard]] int length() const noexcept { return steps.size(); }
};

/**
 * @brief Par de dependência: busId bloqueado por blockerId.
 *
 * blockerId pode ser o id de um Bus ou de um Passenger (tipos distintos
 * são diferenciados por blockerIsBus).
 */
struct BlockDependency
{
    int  blockedBusId  { -1 };
    int  blockerId     { -1 };
    bool blockerIsBus  { true };  ///< false se o bloqueador for um Passenger.
    int  blockerRow    { -1 };
    int  blockerCol    { -1 };
};

/**
 * @brief Sugestão de hint: qual Bus o jogador deve mover a seguir.
 */
struct HintSuggestion
{
    int  busId          { -1 };
    int  unlocksCount   { 0 };    ///< Quantos Buses ficam desbloqueados se este se mover.
    int  distanceToPlat { 0 };    ///< Passos até à plataforma (tie-break).
    bool isValid        { false };
};

// ─────────────────────────────────────────────────────────────────────────────
// Classe MovementSolver
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Analisador de movimentos e pathfinder do Bus Jam Puzzle.
 *
 * Todos os métodos são const e recebem o Board como parâmetro.
 * A instância não guarda estado — pode ser reutilizada para múltiplos níveis.
 */
class MovementSolver
{
public:
    // ─── Construção ────────────────────────────────────────────────────────

    MovementSolver()  = default;
    ~MovementSolver() = default;

    // Copiável e movível — não tem estado membro.
    MovementSolver(const MovementSolver&)            = default;
    MovementSolver& operator=(const MovementSolver&) = default;
    MovementSolver(MovementSolver&&)                 = default;
    MovementSolver& operator=(MovementSolver&&)      = default;

    // ─── Verificações de movimento individual ─────────────────────────────

    /**
     * @brief Verifica se o caminho de um Bus até à sua plataforma está livre.
     *
     * Percorre todas as células entre a posição atual e a plataforma destino.
     * Uma célula é "livre" se OccupancyType == None, ou se for a plataforma
     * destino (em estado Free).
     *
     * @param board Board a analisar.
     * @param busId Id do Bus a verificar.
     * @return true se o Bus pode mover-se diretamente até à plataforma.
     */
    [[nodiscard]] bool isPathClear(const Board& board, int busId) const;

    /**
     * @brief Verifica se a célula imediatamente à frente do Bus está livre.
     *
     * Verificação mais leve do que isPathClear() — apenas um passo.
     * Útil para highlight visual: se apenas o próximo passo estiver livre
     * mas o caminho completo não, o Bus não pode completar o trajeto agora.
     *
     * @param board Board a analisar.
     * @param busId Id do Bus.
     * @return true se a célula seguinte está livre.
     */
    [[nodiscard]] bool canAdvanceOneStep(const Board& board, int busId) const;

    /**
     * @brief Calcula o caminho completo de um Bus até à sua plataforma.
     *
     * Produz a lista de posições intermédias que o Bus vai ocupar,
     * da posição atual (exclusive) até à plataforma (inclusive).
     *
     * @param board Board a analisar.
     * @param busId Id do Bus.
     * @return BusPath com a sequência de posições; isValid=false se
     *         não houver plataforma compatível ou o Bus não existir.
     */
    [[nodiscard]] BusPath calculatePath(const Board& board, int busId) const;

    // ─── Análise global do tabuleiro ──────────────────────────────────────

    /**
     * @brief Devolve os ids de todos os Buses que podem mover-se agora.
     *
     * Um Bus "pode mover-se" se isPathClear() retornar true.
     *
     * @param board Board a analisar.
     * @return Lista de ids (pode ser vazia se nenhum Bus se pode mover).
     */
    [[nodiscard]] QList<int> findMovableBuses(const Board& board) const;

    /**
     * @brief Devolve os ids de todos os Buses ativos que estão bloqueados.
     *
     * Um Bus "está bloqueado" se !isPathClear() e isActive() e !hasLeft().
     *
     * @param board Board a analisar.
     * @return Lista de ids de Buses bloqueados.
     */
    [[nodiscard]] QList<int> findBlockedBuses(const Board& board) const;

    /**
     * @brief Constrói o grafo de dependências de bloqueio.
     *
     * Para cada Bus bloqueado, identifica a entidade que o bloqueia
     * na sua posição imediata (próxima célula na direção de movimento).
     *
     * @param board Board a analisar.
     * @return Lista de BlockDependency; pode ser vazia se nenhum Bus estiver bloqueado.
     */
    [[nodiscard]] QList<BlockDependency> buildDependencyGraph(const Board& board) const;

    /**
     * @brief Verifica se o tabuleiro está em deadlock.
     *
     * Deadlock: nenhum Bus pode mover-se E existem ainda Buses ativos
     * com Passengers por embarcar. Indica que o nível é insolúvel no
     * estado atual (mas não necessariamente insolúvel — Undo pode sair do deadlock).
     *
     * @param board Board a analisar.
     * @return true se estiver em deadlock.
     */
    [[nodiscard]] bool isDeadlocked(const Board& board) const;

    // ─── Sistema de hints ─────────────────────────────────────────────────

    /**
     * @brief Calcula a melhor sugestão de hint para o jogador.
     *
     * Heurística (por prioridade):
     *  1. Bus que pode mover-se imediatamente (isPathClear == true).
     *  2. Entre os que podem mover-se: o que desbloqueia mais outros.
     *  3. Tie-break: o mais próximo da plataforma.
     *
     * @param board Board a analisar.
     * @return HintSuggestion; isValid=false se nenhum Bus se puder mover.
     */
    [[nodiscard]] HintSuggestion suggestHint(const Board& board) const;

    /**
     * @brief Conta quantos Buses seriam desbloqueados se um dado Bus se movesse.
     *
     * Análise especulativa: simula a remoção do Bus da grelha e conta
     * quantos Buses bloqueados ficariam com caminho livre.
     *
     * @param board Board a analisar (não é mutado).
     * @param busId Id do Bus cuja remoção é simulada.
     * @return Número de Buses desbloqueados pela remoção.
     */
    [[nodiscard]] int countUnlockedByMoving(const Board& board, int busId) const;

    /**
     * @brief Devolve a distância em passos de um Bus até à sua plataforma.
     *
     * Conta as células entre a posição atual e a plataforma, na direção
     * de movimento, independentemente de estarem livres ou não.
     *
     * @param board Board a analisar.
     * @param busId Id do Bus.
     * @return Número de passos; -1 se não houver plataforma compatível.
     */
    [[nodiscard]] int distanceToTargetPlatform(const Board& board, int busId) const;

    // ─── Validação ─────────────────────────────────────────────────────────

    /**
     * @brief Verifica se o Board está num estado consistente para análise.
     *
     * Verificações:
     *  - Todos os Buses ativos têm posições dentro dos limites.
     *  - Todos os Passengers em Waiting têm posições dentro dos limites.
     *  - A grelha de ocupação é consistente com as posições das entidades.
     *
     * @param board Board a verificar.
     * @return true se o estado é consistente.
     */
    [[nodiscard]] bool validateBoardState(const Board& board) const;

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual da análise atual para logs e depuração.
     *
     * Formato:
     * "MovementSolver: board=8x8 movable=2 blocked=1 deadlocked=false"
     */
    [[nodiscard]] QString analysisReport(const Board& board) const;

private:
    // ─── Auxiliares privados ──────────────────────────────────────────────

    /**
     * @brief Calcula o delta de posição para a direção de um Bus.
     *
     * @param dir Direção.
     * @return QPoint(deltaCol, deltaRow).
     */
    [[nodiscard]] static QPoint directionDelta(Direction dir) noexcept;

    /**
     * @brief Verifica se uma célula está livre para o Bus passar.
     *
     * Uma célula é "passável" por um Bus se:
     *  - Estiver vazia (OccupancyType::None), OU
     *  - For a plataforma destino do Bus (Platform com mesma cor, estado Free).
     *
     * @param board      Board a analisar.
     * @param pos        Posição a verificar.
     * @param bus        Bus que quer passar.
     * @param targetPlat Plataforma destino do Bus.
     * @return true se passável.
     */
    [[nodiscard]] static bool isCellPassable(const Board&    board,
                                              QPoint          pos,
                                              const Bus&      bus,
                                              const Platform* targetPlat) noexcept;

    /**
     * @brief Encontra a plataforma destino de um Bus no Board.
     *
     * Delegação a Board::findTargetPlatform — centraliza o acesso
     * para facilitar substituição futura por cache de lookup.
     *
     * @param board Board a consultar.
     * @param bus   Bus cujo destino se procura.
     * @return Ponteiro const para Platform, ou nullptr.
     */
    [[nodiscard]] static const Platform* findTarget(const Board& board,
                                                     const Bus&   bus);
};
