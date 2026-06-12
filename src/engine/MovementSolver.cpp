/**
 * @file MovementSolver.cpp
 * @brief Implementação da classe MovementSolver para o jogo Bus Jam Puzzle.
 *
 * Todos os métodos são puramente de leitura: recebem Board const& e
 * nunca chamam métodos mutantes. Isto garante que o solver pode ser
 * invocado a qualquer momento sem risco de corromper o estado do jogo.
 *
 * Convenção de coordenadas (idêntica ao resto do projeto):
 *   QPoint(x=col, y=row) — x aumenta para a direita, y aumenta para baixo.
 *
 * Limite de segurança em loops de percurso de caminho:
 *   maxSteps = board.rows() + board.cols()
 *   Previne loops infinitos caso haja bug de alinhamento Bus↔Platform.
 */

#include "MovementSolver.h"

#include "../model/Passenger.h"
#include <QDebug>
#include <algorithm>   // std::sort, std::max_element

// ─────────────────────────────────────────────────────────────────────────────
// Verificações de movimento individual
// ─────────────────────────────────────────────────────────────────────────────

bool MovementSolver::isPathClear(const Board& board, int busId) const
{
    const Bus* bus = board.findBusById(busId);

    if (!bus || !bus->isActive() || bus->hasLeft())
        return false;

    const Platform* target = findTarget(board, *bus);

    if (!target)
        return false;

    // Se a plataforma já está ocupada ou concluída, não pode receber este Bus.
    if (!target->isFree())
        return false;

    const QPoint busPos      = bus->position();
    const QPoint platformPos = target->position();
    const QPoint delta       = directionDelta(bus->direction());
    const int    maxSteps    = board.rows() + board.cols();

    QPoint current = busPos + delta;  // Começa na célula seguinte.
    int    steps   = 0;

    while (current != platformPos && steps < maxSteps)
    {
        if (!board.isValidPosition(current))
            return false;

        if (!isCellPassable(board, current, *bus, target))
            return false;

        current += delta;
        ++steps;
    }

    // Confirma que chegámos à plataforma (e não saímos por maxSteps).
    return current == platformPos;
}

bool MovementSolver::canAdvanceOneStep(const Board& board, int busId) const
{
    const Bus* bus = board.findBusById(busId);

    if (!bus || !bus->isActive() || bus->hasLeft())
        return false;

    const QPoint next = bus->nextPosition();

    if (!board.isValidPosition(next))
        return false;

    const Platform* target = findTarget(board, *bus);
    return isCellPassable(board, next, *bus, target);
}

BusPath MovementSolver::calculatePath(const Board& board, int busId) const
{
    BusPath path;
    path.busId = busId;

    const Bus* bus = board.findBusById(busId);

    if (!bus || !bus->isActive() || bus->hasLeft())
        return path;

    const Platform* target = findTarget(board, *bus);

    if (!target)
        return path;

    path.platformId = target->id();

    const QPoint platformPos = target->position();
    const QPoint delta       = directionDelta(bus->direction());
    const int    maxSteps    = board.rows() + board.cols();

    QPoint current = bus->position() + delta;
    int    steps   = 0;

    while (current != platformPos && steps < maxSteps)
    {
        if (!board.isValidPosition(current))
            return path;  // isValid permanece false.

        path.steps.append(current);
        current += delta;
        ++steps;
    }

    if (current != platformPos)
        return path;  // Limite de segurança atingido sem chegar ao destino.

    // Inclui a posição da plataforma como último passo.
    path.steps.append(platformPos);
    path.isValid = true;

    return path;
}

// ─────────────────────────────────────────────────────────────────────────────
// Análise global do tabuleiro
// ─────────────────────────────────────────────────────────────────────────────

QList<int> MovementSolver::findMovableBuses(const Board& board) const
{
    QList<int> movable;

    for (const auto& busPtr : board.buses())
    {
        const Bus* bus = busPtr.get();

        if (!bus->isActive() || bus->hasLeft())
            continue;

        if (isPathClear(board, bus->id()))
            movable.append(bus->id());
    }

    return movable;
}

QList<int> MovementSolver::findBlockedBuses(const Board& board) const
{
    QList<int> blocked;

    for (const auto& busPtr : board.buses())
    {
        const Bus* bus = busPtr.get();

        if (!bus->isActive() || bus->hasLeft())
            continue;

        if (!isPathClear(board, bus->id()))
            blocked.append(bus->id());
    }

    return blocked;
}

QList<BlockDependency> MovementSolver::buildDependencyGraph(const Board& board) const
{
    QList<BlockDependency> deps;

    for (const auto& busPtr : board.buses())
    {
        const Bus* bus = busPtr.get();

        if (!bus->isActive() || bus->hasLeft())
            continue;

        if (isPathClear(board, bus->id()))
            continue;  // Não está bloqueado.

        // Inspecionar a próxima célula para encontrar o bloqueador imediato.
        const QPoint nextPos = bus->nextPosition();

        if (!board.isValidPosition(nextPos))
            continue;

        const CellInfo info = board.cellInfoAt(nextPos);

        if (info.type == OccupancyType::None || info.type == OccupancyType::Platform)
            continue;  // Célula livre ou plataforma — o bloqueio é mais distante.

        BlockDependency dep;
        dep.blockedBusId = bus->id();
        dep.blockerId    = info.id;
        dep.blockerIsBus = (info.type == OccupancyType::BusEntity);
        dep.blockerRow   = nextPos.y();
        dep.blockerCol   = nextPos.x();

        deps.append(dep);
    }

    return deps;
}

bool MovementSolver::isDeadlocked(const Board& board) const
{
    // Deadlock: nenhum Bus pode mover-se E há ainda Passengers por embarcar.
    if (board.waitingPassengerCount() == 0)
        return false;  // Todos os Passengers já embarcaram — não é deadlock.

    if (board.activeBusCount() == 0)
        return false;  // Não há Buses ativos — não é deadlock (é vitória ou estado inválido).

    return findMovableBuses(board).isEmpty();
}

// ─────────────────────────────────────────────────────────────────────────────
// Sistema de hints
// ─────────────────────────────────────────────────────────────────────────────

HintSuggestion MovementSolver::suggestHint(const Board& board) const
{
    HintSuggestion best;

    const QList<int> movable = findMovableBuses(board);

    if (movable.isEmpty())
        return best;  // isValid=false — nenhum Bus pode mover-se.

    for (int busId : movable)
    {
        const int unlocks  = countUnlockedByMoving(board, busId);
        const int distance = distanceToTargetPlatform(board, busId);

        // Substitui o melhor se:
        //  - desbloqueia mais Buses, OU
        //  - igual desbloqueio mas mais próximo da plataforma.
        const bool isBetter = !best.isValid
            || unlocks > best.unlocksCount
            || (unlocks == best.unlocksCount && distance < best.distanceToPlat);

        if (isBetter)
        {
            best.busId          = busId;
            best.unlocksCount   = unlocks;
            best.distanceToPlat = distance;
            best.isValid        = true;
        }
    }

    return best;
}

int MovementSolver::countUnlockedByMoving(const Board& board, int busId) const
{
    const Bus* bus = board.findBusById(busId);

    if (!bus || !bus->isActive() || bus->hasLeft())
        return 0;

    // Recolher os Buses bloqueados antes da remoção.
    const QList<int> blockedBefore = findBlockedBuses(board);

    if (blockedBefore.isEmpty())
        return 0;

    // Simular a remoção do Bus: quais células ele ocupava?
    // Verificamos quais dos Buses bloqueados teriam o caminho livre
    // se o Bus alvo não estivesse no tabuleiro.
    //
    // Implementação: para cada Bus bloqueado, verificar se o único
    // obstáculo no seu caminho é o Bus alvo.
    // Isto evita ter de clonar o Board completo.

    const QPoint removedPos = bus->position();
    int          unlocked   = 0;

    for (int blockedId : blockedBefore)
    {
        if (blockedId == busId)
            continue;

        const Bus* blocked = board.findBusById(blockedId);
        if (!blocked)
            continue;

        const Platform* target = findTarget(board, *blocked);
        if (!target || !target->isFree())
            continue;

        const QPoint delta    = directionDelta(blocked->direction());
        const QPoint startPos = blocked->position() + delta;
        const QPoint endPos   = target->position();
        const int    maxSteps = board.rows() + board.cols();

        QPoint current = startPos;
        int    steps   = 0;
        bool   pathClearWithoutBus = true;

        while (current != endPos && steps < maxSteps)
        {
            if (!board.isValidPosition(current))
            {
                pathClearWithoutBus = false;
                break;
            }

            // Tratar a posição do Bus removido como livre.
            if (current != removedPos)
            {
                if (!isCellPassable(board, current, *blocked, target))
                {
                    pathClearWithoutBus = false;
                    break;
                }
            }

            current += delta;
            ++steps;
        }

        if (pathClearWithoutBus && current == endPos)
            ++unlocked;
    }

    return unlocked;
}

int MovementSolver::distanceToTargetPlatform(const Board& board, int busId) const
{
    const Bus* bus = board.findBusById(busId);

    if (!bus || !bus->isActive() || bus->hasLeft())
        return -1;

    const Platform* target = findTarget(board, *bus);

    if (!target)
        return -1;

    const QPoint busPos  = bus->position();
    const QPoint platPos = target->position();
    const QPoint delta   = directionDelta(bus->direction());

    // Conta os passos percorrendo a direção até à plataforma.
    // Não verifica obstáculos — apenas mede a distância geométrica.
    int     steps   = 0;
    QPoint  current = busPos + delta;
    const int maxSteps = board.rows() + board.cols();

    while (current != platPos && steps < maxSteps)
    {
        current += delta;
        ++steps;
    }

    return (current == platPos) ? steps + 1 : -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Validação
// ─────────────────────────────────────────────────────────────────────────────

bool MovementSolver::validateBoardState(const Board& board) const
{
    bool valid = true;

    // Verificar posições dos Buses ativos.
    for (const auto& busPtr : board.buses())
    {
        const Bus* bus = busPtr.get();

        if (!bus->isActive() || bus->hasLeft())
            continue;

        if (!board.isValidPosition(bus->position()))
        {
            qWarning().noquote()
                << "MovementSolver::validateBoardState: Bus id=" << bus->id()
                << "tem posição inválida" << bus->position();
            valid = false;
        }

        // Verificar consistência com a grelha de ocupação.
        const CellInfo info = board.cellInfoAt(bus->position());
        if (info.type != OccupancyType::BusEntity || info.id != bus->id())
        {
            qWarning().noquote()
                << "MovementSolver::validateBoardState: Bus id=" << bus->id()
                << "em" << bus->position()
                << "não está registado na grelha (tipo="
                << static_cast<int>(info.type) << "id=" << info.id << ")";
            valid = false;
        }
    }

    // Verificar posições dos Passengers em Waiting.
    for (const auto& pPtr : board.passengers())
    {
        const Passenger* p = pPtr.get();

        if (!p->isWaiting())
            continue;

        if (!board.isValidPosition(p->cellPosition()))
        {
            qWarning().noquote()
                << "MovementSolver::validateBoardState: Passenger id=" << p->id()
                << "tem posição inválida" << p->cellPosition();
            valid = false;
        }

        const CellInfo info = board.cellInfoAt(p->cellPosition());
        if (info.type != OccupancyType::Passenger || info.id != p->id())
        {
            qWarning().noquote()
                << "MovementSolver::validateBoardState: Passenger id=" << p->id()
                << "em" << p->cellPosition()
                << "não está registado na grelha (tipo="
                << static_cast<int>(info.type) << "id=" << info.id << ")";
            valid = false;
        }
    }

    return valid;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString MovementSolver::analysisReport(const Board& board) const
{
    const QList<int> movable  = findMovableBuses(board);
    const QList<int> blocked  = findBlockedBuses(board);
    const bool       deadlock = isDeadlocked(board);
    const HintSuggestion hint = suggestHint(board);

    return QString(
        "MovementSolver: board=%1x%2 movable=%3 blocked=%4 "
        "deadlocked=%5 hintBus=%6")
        .arg(board.rows())
        .arg(board.cols())
        .arg(movable.size())
        .arg(blocked.size())
        .arg(deadlock ? "true" : "false")
        .arg(hint.isValid ? hint.busId : -1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

QPoint MovementSolver::directionDelta(Direction dir) noexcept
{
    // Idêntico a Bus::directionDelta — quando Enums.h for extraído,
    // ambos usarão a mesma função global.
    switch (dir)
    {
        case Direction::Up:    return QPoint( 0, -1);
        case Direction::Down:  return QPoint( 0,  1);
        case Direction::Left:  return QPoint(-1,  0);
        case Direction::Right: return QPoint( 1,  0);
    }
    Q_UNREACHABLE();
    return QPoint(0, 0);
}

bool MovementSolver::isCellPassable(const Board&    board,
                                     QPoint          pos,
                                     const Bus&      bus,
                                     const Platform* targetPlat)
{
    const CellInfo info = board.cellInfoAt(pos);

    if (info.type == OccupancyType::None)
        return true;

    // A plataforma destino (mesma cor, Free) é passável pelo Bus.
    if (info.type == OccupancyType::Platform && targetPlat)
    {
        return info.id == targetPlat->id()
            && targetPlat->color() == bus.color()
            && targetPlat->isFree();
    }

    // Passageiros da mesma cor em Waiting são recolhidos em trânsito — não bloqueiam.
    if (info.type == OccupancyType::Passenger)
    {
        const Passenger* p = board.findPassengerById(info.id);
        return p != nullptr && p->color() == bus.color() && p->isWaiting();
    }

    // Qualquer outro ocupante (Bus, outra Platform) é obstáculo.
    return false;
}

const Platform* MovementSolver::findTarget(const Board& board, const Bus& bus)
{
    return board.findTargetPlatform(bus.id());
}
