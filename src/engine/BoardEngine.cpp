/**
 * @file BoardEngine.cpp
 * @brief Implementação da classe BoardEngine para o jogo Bus Jam Puzzle.
 *
 * Todos os movimentos passam por requestBusMove() → validateMoveRequest()
 * → captureSnapshot() → executeFullMove() → checkAndHandleVictory().
 *
 * Invariante central: state_ == Processing durante executeFullMove().
 * Isto impede reentrada (ex.: callback que tenta mover outro Bus).
 * Após executeFullMove(), state_ volta a Ready ou transita para Won.
 *
 * Política de erro:
 *  - requestBusMove() devolve MoveResult; nunca lança exceção.
 *  - Métodos internos usam qWarning + return false para erros inesperados.
 *  - Q_ASSERT para invariantes que não devem ser violados em produção.
 */

#include "BoardEngine.h"

#include <QDebug>
#include <algorithm>   // std::min

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────

BoardEngine::BoardEngine(Board& board)
    : board_     { board }
    , state_     { EngineState::Idle }
    , moveCount_ { 0 }
{
    // O engine só está pronto se o Board tiver entidades suficientes.
    if (board_.busCount() > 0 && board_.platformCount() > 0)
    {
        state_ = EngineState::Ready;
    }
    else
    {
        qWarning().noquote()
            << "BoardEngine: Board insuficiente (buses="
            << board_.busCount()
            << "platforms=" << board_.platformCount()
            << "). Estado: Idle.";
    }

    qDebug().noquote()
        << "BoardEngine criado. Estado=" << stateName()
        << board_.toString();
}

// ─────────────────────────────────────────────────────────────────────────────
// Estado do engine
// ─────────────────────────────────────────────────────────────────────────────

EngineState BoardEngine::state() const noexcept { return state_; }

QString BoardEngine::stateName() const
{
    return QString::fromLatin1(BoardEngine::engineStateName(state_));
}

bool BoardEngine::isReady() const noexcept { return state_ == EngineState::Ready; }
bool BoardEngine::isWon()   const noexcept { return state_ == EngineState::Won;   }

int BoardEngine::moveCount() const noexcept { return moveCount_; }

const Board& BoardEngine::board() const noexcept { return board_; }

// ─────────────────────────────────────────────────────────────────────────────
// Ação principal: pedido de movimento
// ─────────────────────────────────────────────────────────────────────────────

MoveResult BoardEngine::requestBusMove(int busId)
{
    // ── Passo 1: validação de pré-condições ──────────────────────────────
    Bus*       bus    = nullptr;
    MoveResult result = MoveResult::InvalidState;

    if (!validateMoveRequest(busId, &bus, result))
    {
        qWarning().noquote()
            << "BoardEngine::requestBusMove id=" << busId
            << "→" << moveResultName(result);
        return result;
    }

    Q_ASSERT(bus != nullptr);

    // ── Passo 2: localizar plataforma destino ────────────────────────────
    Platform* platform = board_.findTargetPlatform(busId);

    if (!platform)
    {
        qWarning().noquote()
            << "BoardEngine::requestBusMove id=" << busId
            << ": nenhuma plataforma compatível encontrada.";
        return MoveResult::NoPlatform;
    }

    // ── Passo 3: verificar caminho livre ─────────────────────────────────
    if (!board_.isPathToplatformClear(busId))
    {
        qDebug().noquote()
            << "BoardEngine::requestBusMove id=" << busId
            << ": caminho bloqueado.";
        return MoveResult::Blocked;
    }

    // ── Passo 4: guardar snapshot para Undo ──────────────────────────────
    pushSnapshot(captureSnapshot());

    // ── Passo 5: marcar engine como Processing (anti-reentrada) ──────────
    const EngineState previousState = state_;
    state_ = EngineState::Processing;

    qDebug().noquote()
        << "BoardEngine: iniciando movimento do Bus id=" << busId
        << "→ Platform id=" << platform->id();

    // ── Passo 6: executar movimento completo ─────────────────────────────
    const bool success = executeFullMove(busId, platform);

    if (!success)
    {
        // Erro inesperado durante execução — restaurar estado anterior.
        // Num jogo de produção, isto deveria disparar um undo automático.
        qWarning().noquote()
            << "BoardEngine::requestBusMove: executeFullMove falhou para Bus id="
            << busId << ". Estado revertido.";
        state_ = previousState;
        return MoveResult::InvalidState;
    }

    ++moveCount_;

    // ── Passo 7: verificar vitória (transita state_ se ganhou) ───────────
    // Se não ganhou, repõe Ready.
    if (!checkAndHandleVictory())
        state_ = EngineState::Ready;

    qDebug().noquote()
        << "BoardEngine: jogada" << moveCount_ << "concluída."
        << toString();

    return MoveResult::Success;
}

// ─────────────────────────────────────────────────────────────────────────────
// Undo
// ─────────────────────────────────────────────────────────────────────────────

bool BoardEngine::canUndo() const noexcept
{
    return !undoStack_.isEmpty();
}

int BoardEngine::undoStackSize() const noexcept
{
    return undoStack_.size();
}

bool BoardEngine::undoLastMove()
{
    // Implementação futura: restaurar o snapshot do topo da pilha.
    // A estrutura de dados (undoStack_, BoardSnapshot, applySnapshot)
    // está pronta. Falta a lógica de aplicação aos objetos de domínio.
    qWarning() << "BoardEngine::undoLastMove: ainda não implementado.";
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Restart
// ─────────────────────────────────────────────────────────────────────────────

void BoardEngine::restart()
{
    qDebug().noquote()
        << "BoardEngine::restart — a reiniciar nível."
        << "Jogadas anteriores:" << moveCount_;

    board_.reset();
    undoStack_.clear();
    moveCount_ = 0;
    state_     = EngineState::Ready;

    qDebug().noquote()
        << "BoardEngine::restart — concluído. " << board_.toString();
}

// ─────────────────────────────────────────────────────────────────────────────
// Callbacks
// ─────────────────────────────────────────────────────────────────────────────

void BoardEngine::setOnBusMoved(BusMovedCallback cb)
{
    onBusMoved_ = std::move(cb);
}

void BoardEngine::setOnPassengerBoarded(PassengerBoardedCallback cb)
{
    onPassengerBoarded_ = std::move(cb);
}

void BoardEngine::setOnBusLeft(BusLeftCallback cb)
{
    onBusLeft_ = std::move(cb);
}

void BoardEngine::setOnLevelComplete(LevelCompleteCallback cb)
{
    onLevelComplete_ = std::move(cb);
}

// ─────────────────────────────────────────────────────────────────────────────
// Consultas de estado do jogo
// ─────────────────────────────────────────────────────────────────────────────

int BoardEngine::remainingPassengers() const
{
    return board_.waitingPassengerCount();
}

int BoardEngine::remainingBuses() const
{
    return board_.activeBusCount();
}

bool BoardEngine::canBusMove(int busId) const
{
    if (!isReady())
        return false;

    const Bus* bus = board_.findBusById(busId);

    if (!bus || !bus->isActive() || bus->hasLeft())
        return false;

    return board_.isPathToplatformClear(busId);
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString BoardEngine::toString() const
{
    return QString("BoardEngine[state=%1 moves=%2 undoStack=%3 won=%4]")
        .arg(stateName())
        .arg(moveCount_)
        .arg(undoStack_.size())
        .arg(isWon() ? "true" : "false");
}

// ─────────────────────────────────────────────────────────────────────────────
// Lógica de movimento (privada)
// ─────────────────────────────────────────────────────────────────────────────

bool BoardEngine::executeFullMove(int busId, Platform* platform)
{
    Q_ASSERT(platform != nullptr);

    Bus* bus = board_.findBusById(busId);

    if (!bus)
    {
        qWarning().noquote()
            << "BoardEngine::executeFullMove: Bus id=" << busId << "não encontrado.";
        return false;
    }

    const QPoint platformPos = platform->position();

    // ── Fase 0.5: limpar passageiros da mesma cor no caminho ────────────
    // Os passageiros permanecem em Waiting; boardPassengers faz as transições.
    freePassengersOnPath(busId, platform);

    // ── Fase 1: avançar o Bus até à plataforma ───────────────────────────
    if (!advanceBusToPosition(busId, platformPos))
    {
        qWarning().noquote()
            << "BoardEngine::executeFullMove: falha ao avançar Bus id="
            << busId << "até" << platformPos;
        return false;
    }

    // ── Fase 2: registar chegada na plataforma ───────────────────────────
    bus->setAtPlatform(true);

    if (!platform->receberBus(busId, bus->color()))
    {
        qWarning().noquote()
            << "BoardEngine::executeFullMove: plataforma id=" << platform->id()
            << "recusou Bus id=" << busId;
        bus->setAtPlatform(false);
        return false;
    }

    qDebug().noquote()
        << "BoardEngine: Bus id=" << busId
        << "chegou à plataforma id=" << platform->id();

    // ── Fase 3: embarcar passageiros ─────────────────────────────────────
    const int boarded = boardPassengers(bus, platform);

    qDebug().noquote()
        << "BoardEngine: Bus id=" << busId
        << "embarcou" << boarded << "passageiro(s).";

    // ── Fase 4: sair do tabuleiro ─────────────────────────────────────────
    if (!bus->sairDoTabuleiro())
    {
        qWarning().noquote()
            << "BoardEngine::executeFullMove: Bus id=" << busId
            << "falhou ao sair do tabuleiro.";
        return false;
    }

    // Liberta a célula da plataforma na grelha (autocarro saiu).
    // A plataforma em si permanece na grelha como Platform.
    // Usamos freeCell na posição do Bus ANTES de sairDoTabuleiro limpar a posição.
    // Como sairDoTabuleiro já marcou position como InvalidPosition, usamos platformPos.
    board_.freeCell(platformPos);
    // Repõe a marca de Platform na grelha (freeCell limpa para None).
    board_.occupyCell(platformPos, OccupancyType::Platform, platform->id());

    if (!platform->concluirPlatform())
    {
        qWarning().noquote()
            << "BoardEngine::executeFullMove: falha ao concluir plataforma id="
            << platform->id();
        // Não é fatal — o Bus já saiu. Continua.
    }

    bus->setAtPlatform(false);  // Já saiu; limpa flag por consistência.

    // Invoca callback de saída.
    if (onBusLeft_)
        onBusLeft_(busId);

    qDebug().noquote()
        << "BoardEngine: Bus id=" << busId << "saiu do tabuleiro."
        << board_.toString();

    return true;
}

bool BoardEngine::advanceBusToPosition(int busId, QPoint target)
{
    Bus* bus = board_.findBusById(busId);

    if (!bus)
        return false;

    // Limite de segurança: o número máximo de passos é o perímetro do tabuleiro.
    // Previne loops infinitos em caso de bug de alinhamento.
    const int maxSteps = board_.rows() + board_.cols();
    int       steps    = 0;

    while (bus->position() != target)
    {
        if (steps >= maxSteps)
        {
            qWarning().noquote()
                << "BoardEngine::advanceBusToPosition: Bus id=" << busId
                << "excedeu o número máximo de passos (" << maxSteps
                << "). Possível loop. Posição atual=" << bus->position()
                << "Target=" << target;
            return false;
        }

        const QPoint fromPos = bus->position();
        const QPoint toPos   = bus->nextPosition();

        // Valida o próximo passo antes de executar.
        if (!board_.isValidPosition(toPos))
        {
            qWarning().noquote()
                << "BoardEngine::advanceBusToPosition: Bus id=" << busId
                << "tentaria sair dos limites em" << toPos;
            return false;
        }

        // Executa o deslocamento no modelo.
        if (!bus->mover())
        {
            qWarning().noquote()
                << "BoardEngine::advanceBusToPosition: Bus::mover() falhou"
                << "para Bus id=" << busId;
            return false;
        }

        // Actualiza a grelha de ocupação.
        if (!board_.moveBusOnGrid(busId, fromPos, toPos))
        {
            qWarning().noquote()
                << "BoardEngine::advanceBusToPosition: moveBusOnGrid falhou"
                << "de" << fromPos << "para" << toPos;
            // Bus::mover() já executou — estado inconsistente.
            // Em produção, isto dispararia um undo automático.
            return false;
        }

        // Notifica o GameController / camada de apresentação.
        if (onBusMoved_)
            onBusMoved_(busId, fromPos, toPos);

        ++steps;
    }

    qDebug().noquote()
        << "BoardEngine: Bus id=" << busId
        << "chegou a" << target << "em" << steps << "passo(s).";

    return true;
}

int BoardEngine::boardPassengers(Bus* bus, Platform* platform)
{
    Q_ASSERT(bus != nullptr);
    Q_ASSERT(platform != nullptr);

    // Verificação defensiva de estado.
    if (!bus->isAtPlatform() || !platform->isOccupied())
    {
        qWarning().noquote()
            << "BoardEngine::boardPassengers: Bus id=" << bus->id()
            << "ou Platform id=" << platform->id()
            << "em estado inválido para embarque.";
        return 0;
    }

    int boardedCount = 0;

    // Itera sobre todos os passageiros. A lista não é modificada durante
    // a iteração (passageiros não são removidos, apenas mudam de estado).
    for (const auto& passengerPtr : board_.passengers())
    {
        Passenger* passenger = passengerPtr.get();

        // Apenas passageiros em Waiting e compatíveis com este autocarro.
        if (!passenger->isCompatibleWith(bus->color()))
            continue;

        // Verifica se o autocarro ainda tem lugar.
        if (!bus->podeEmbarcar(passenger->color()))
            break;  // Bus cheio — inutilmente verificar os restantes.

        // ── Ciclo de embarque ────────────────────────────────────────────

        // Passo 1: iniciar embarque no Passenger.
        if (!passenger->iniciarEmbarque(bus->id(), bus->color()))
        {
            qWarning().noquote()
                << "BoardEngine::boardPassengers: Passenger id="
                << passenger->id() << "recusou iniciarEmbarque.";
            continue;
        }

        // Passo 2: registar no Bus.
        if (!bus->embarcarPassageiro(passenger->color()))
        {
            qWarning().noquote()
                << "BoardEngine::boardPassengers: Bus id=" << bus->id()
                << "recusou embarcarPassageiro para Passenger id="
                << passenger->id();
            // Tentativa de consistência: reverter initiation (best-effort).
            // Na versão atual, Passenger fica em Boarding — undo cobrirá este caso.
            continue;
        }

        // Passo 3: concluir embarque no Passenger.
        if (!passenger->concluirEmbarque())
        {
            qWarning().noquote()
                << "BoardEngine::boardPassengers: Passenger id="
                << passenger->id() << "falhou concluirEmbarque.";
            continue;
        }

        // Passo 4: libertar a célula do passageiro na grelha (se ainda ocupada).
        // A célula pode já ter sido libertada por freePassengersOnPath antes
        // do movimento do Bus; nesse caso não há nada a fazer.
        const QPoint pPos = passenger->cellPosition();
        if (board_.cellInfoAt(pPos).id == passenger->id())
            board_.freeCell(pPos);

        ++boardedCount;

        // Notifica o GameController.
        if (onPassengerBoarded_)
            onPassengerBoarded_(passenger->id(), bus->id());
    }

    return boardedCount;
}

bool BoardEngine::checkAndHandleVictory()
{
    if (!board_.isLevelComplete())
        return false;

    state_ = EngineState::Won;

    qDebug().noquote()
        << "BoardEngine: NÍVEL CONCLUÍDO em" << moveCount_ << "jogada(s)!";

    if (onLevelComplete_)
        onLevelComplete_();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Gestão do histórico de Undo
// ─────────────────────────────────────────────────────────────────────────────

BoardSnapshot BoardEngine::captureSnapshot() const
{
    BoardSnapshot snapshot;
    snapshot.moveNumber = moveCount_;

    // Captura estado de todos os autocarros.
    for (const auto& busPtr : board_.buses())
    {
        const Bus* bus = busPtr.get();
        snapshot.buses.push_back(BusSnapshot{
            bus->id(),
            bus->position(),
            bus->boardedCount(),
            bus->isActive(),
            bus->hasLeft(),
            bus->isAtPlatform()
        });
    }

    // Captura estado de todos os passageiros.
    for (const auto& passengerPtr : board_.passengers())
    {
        const Passenger* p = passengerPtr.get();
        snapshot.passengers.push_back(PassengerSnapshot{
            p->id(),
            p->state(),
            p->busId()
        });
    }

    // Captura estado de todas as plataformas.
    for (const auto& platformPtr : board_.platforms())
    {
        const Platform* pl = platformPtr.get();
        snapshot.platforms.push_back(PlatformSnapshot{
            pl->id(),
            pl->state(),
            pl->currentBusId()
        });
    }

    return snapshot;
}

void BoardEngine::pushSnapshot(BoardSnapshot snapshot)
{
    // Se a pilha atingiu o limite, descarta o snapshot mais antigo.
    if (undoStack_.size() >= BoardEngineConstants::kMaxUndoHistory)
    {
        undoStack_.removeFirst();
    }

    undoStack_.push_back(std::move(snapshot));
}

void BoardEngine::applySnapshot(const BoardSnapshot& snapshot)
{
    // Implementação futura.
    // Para cada BusSnapshot em snapshot.buses:
    //   Bus* bus = board_.findBusById(s.id);
    //   bus->setPosition(s.position);
    //   ... restaurar boardedCount, isActive, hasLeft, isAtPlatform ...
    // Idem para Passenger e Platform.
    // Após restaurar entidades, chamar board_.rebuildGrids() (quando exposto).
    Q_UNUSED(snapshot);
    qWarning() << "BoardEngine::applySnapshot: ainda não implementado.";
}

// ─────────────────────────────────────────────────────────────────────────────
// Recolha de passageiros em trânsito
// ─────────────────────────────────────────────────────────────────────────────

void BoardEngine::freePassengersOnPath(int busId, const Platform* platform)
{
    const Bus* bus = board_.findBusById(busId);

    if (!bus || !platform)
        return;

    const QPoint busPos      = bus->position();
    const QPoint platformPos = platform->position();
    const QPoint delta       = QPoint(
        (platformPos.x() > busPos.x()) ? 1 : (platformPos.x() < busPos.x()) ? -1 : 0,
        (platformPos.y() > busPos.y()) ? 1 : (platformPos.y() < busPos.y()) ? -1 : 0
    );

    QPoint current = busPos + delta;

    while (current != platformPos)
    {
        if (!board_.isValidPosition(current))
            break;

        const CellInfo info = board_.cellInfoAt(current);

        if (info.type == OccupancyType::Passenger)
        {
            const Passenger* p = board_.findPassengerById(info.id);
            if (p && p->color() == bus->color() && p->isWaiting())
            {
                board_.freeCell(current);
                qDebug().noquote()
                    << "BoardEngine: passageiro id=" << info.id
                    << "recolhido em trânsito em" << current;
            }
        }

        current += delta;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares de validação
// ─────────────────────────────────────────────────────────────────────────────

bool BoardEngine::validateMoveRequest(int        busId,
                                      Bus**      outBus,
                                      MoveResult& outResult) const
{
    // Condição 1: engine deve estar pronto.
    if (state_ != EngineState::Ready)
    {
        outResult = MoveResult::InvalidState;
        return false;
    }

    // Condição 2: Bus deve existir.
    Bus* bus = board_.findBusById(busId);

    if (!bus)
    {
        outResult = MoveResult::BusNotFound;
        return false;
    }

    // Condição 3: Bus não pode ter já saído.
    if (bus->hasLeft())
    {
        outResult = MoveResult::AlreadyLeft;
        return false;
    }

    // Condição 4: Bus deve estar ativo.
    if (!bus->isActive())
    {
        outResult = MoveResult::InvalidState;
        return false;
    }

    *outBus   = bus;
    outResult = MoveResult::Success;
    return true;
}

const char* BoardEngine::moveResultName(MoveResult result) noexcept
{
    switch (result)
    {
        case MoveResult::Success:      return "Success";
        case MoveResult::Blocked:      return "Blocked";
        case MoveResult::AlreadyLeft:  return "AlreadyLeft";
        case MoveResult::BusNotFound:  return "BusNotFound";
        case MoveResult::NoPlatform:   return "NoPlatform";
        case MoveResult::InvalidState: return "InvalidState";
    }
    Q_UNREACHABLE();
    return "Unknown";
}

const char* BoardEngine::engineStateName(EngineState state) noexcept
{
    switch (state)
    {
        case EngineState::Ready:      return "Ready";
        case EngineState::Processing: return "Processing";
        case EngineState::Won:        return "Won";
        case EngineState::Idle:       return "Idle";
    }
    Q_UNREACHABLE();
    return "Unknown";
}
