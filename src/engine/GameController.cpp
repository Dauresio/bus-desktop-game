/**
 * @file GameController.cpp
 * @brief Implementação da classe GameController — versão final com List Models.
 *
 * Cada callback do BoardEngine agora tem duas responsabilidades:
 *  1. Actualizar o modelo de lista correspondente (para o Repeater QML).
 *  2. Emitir o sinal Qt correspondente (para animações no QML).
 *
 * A separação é intencional: o modelo de lista mantém o estado persistente
 * (posição, estado de embarque, etc.) enquanto o sinal é efémero e serve
 * apenas para accionar animações de transição.
 *
 * ### Sequência de embarque de passageiro:
 *
 *  onPassengerBoarded_(passengerId, busId):
 *    1. passengerListModel_->updatePassenger(id, boarding=true, boarded=false)
 *       → PassengerItem.qml activa a animação de boarding (encolhe)
 *    2. busListModel_->updateBus(busId, …, boardedCount+1, …)
 *       → BusItem.qml actualiza a barra de lotação
 *    3. emit passengerBoarded(passengerId, busId)
 *       → GameScreen.qml pode reagir (som, vibração)
 *    4. QTimer::singleShot(kBoardingAnimMs):
 *       passengerListModel_->updatePassenger(id, boarding=false, boarded=true)
 *       → PassengerItem.qml fica invisible (isBoarded=true)
 *
 * ### Nota sobre plataformas:
 *  O BoardEngine não tem (ainda) um callback onPlatformChanged_.
 *  O estado da plataforma é inferido a partir dos callbacks existentes:
 *  - receberBus   → Occupied  (detectado em onBusMoved_ quando Bus chega à plataforma)
 *  - concluirPlat → Completed (detectado em onBusLeft_)
 *  A actualização correcta é feita consultando o Board após cada evento.
 */

#include "GameController.h"
#include "LevelManager.h"
#include "PersistenceManager.h"

#include <QDebug>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Registo de metatypes
// ─────────────────────────────────────────────────────────────────────────────

namespace {
    const bool kMetatypesRegistered = []() {
        qRegisterMetaType<GameState>("GameState");
        qRegisterMetaType<MoveOutcome>("MoveOutcome");
        qRegisterMetaType<LevelResult>("LevelResult");
        return true;
    }();
}

// ─────────────────────────────────────────────────────────────────────────────
// Construtor e destrutor
// ─────────────────────────────────────────────────────────────────────────────

GameController::GameController(LevelManager*       levelManager,
                               PersistenceManager* persistenceManager,
                               QObject*            parent)
    : QObject              { parent }
    , levelManager_        { levelManager }
    , persistenceManager_  { persistenceManager }
    // Os modelos são filhos QObject — destruídos automaticamente com o controller.
    , busListModel_        { new BusListModel(this)       }
    , passengerListModel_  { new PassengerListModel(this) }
    , platformListModel_   { new PlatformListModel(this)  }
    , gameState_           { GameState::Menu }
    , currentLevelId_      { -1 }
    , highestUnlockedLevel_{ 1 }
    , hintsRemaining_      { 3 }
    , currentBoard_        { nullptr }
    , currentEngine_       { nullptr }
    , elapsedSeconds_      { 0 }
    , timeLimitSecs_       { GameControllerConstants::kDefaultTimeLimitSecs }
    , currentScore_        { 0 }
{
    gameTimer_.setInterval(GameControllerConstants::kTimerIntervalMs);
    gameTimer_.setSingleShot(false);
    connect(&gameTimer_, &QTimer::timeout, this, &GameController::onTimerTick);

    loadProgress();

    qDebug().noquote() << "GameController criado." << toString();
}

GameController::~GameController()
{
    gameTimer_.stop();
    qDebug() << "GameController destruído.";
}

// ─────────────────────────────────────────────────────────────────────────────
// Leitores de Q_PROPERTY
// ─────────────────────────────────────────────────────────────────────────────

GameState GameController::gameState()            const noexcept { return gameState_; }
int       GameController::currentLevelId()       const noexcept { return currentLevelId_; }
int       GameController::moveCount()            const noexcept { return currentEngine_ ? currentEngine_->moveCount() : 0; }
int       GameController::elapsedSeconds()       const noexcept { return elapsedSeconds_; }
int       GameController::currentScore()         const noexcept { return currentScore_; }
int       GameController::highestUnlockedLevel() const noexcept { return highestUnlockedLevel_; }
bool      GameController::canUndo()              const noexcept { return currentEngine_ ? currentEngine_->canUndo() : false; }
bool      GameController::canHint()              const noexcept { return hintsRemaining_ > 0 && gameState_ == GameState::Playing; }

int GameController::remainingSeconds() const noexcept
{
    if (timeLimitSecs_ <= 0) return 0;
    return std::max(0, timeLimitSecs_ - elapsedSeconds_);
}

int GameController::remainingPassengers() const noexcept
{
    return currentEngine_ ? currentEngine_->remainingPassengers() : 0;
}

int GameController::remainingBuses() const noexcept
{
    return currentEngine_ ? currentEngine_->remainingBuses() : 0;
}

// ─── Modelos de lista ─────────────────────────────────────────────────────

QAbstractListModel* GameController::busModel()       const noexcept { return busListModel_; }
QAbstractListModel* GameController::passengerModel() const noexcept { return passengerListModel_; }
QAbstractListModel* GameController::platformModel()  const noexcept { return platformListModel_; }

// ─── Dimensões do tabuleiro ───────────────────────────────────────────────

int GameController::boardRows() const noexcept
{
    return currentBoard_ ? currentBoard_->rows() : 8;
}

int GameController::boardCols() const noexcept
{
    return currentBoard_ ? currentBoard_->cols() : 8;
}

// ─── Outros getters ───────────────────────────────────────────────────────

const BoardEngine* GameController::engine()     const noexcept { return currentEngine_.get(); }
const LevelResult& GameController::lastResult() const noexcept { return lastResult_; }

bool GameController::isLevelUnlocked(int levelId) const noexcept
{
    return levelId >= 1 && levelId <= highestUnlockedLevel_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Q_INVOKABLE
// ─────────────────────────────────────────────────────────────────────────────

int GameController::boardedCountFor(int busId) const
{
    if (!currentBoard_) return 0;
    const Bus* bus = currentBoard_->findBusById(busId);
    return bus ? bus->boardedCount() : 0;
}

int GameController::starsForLevel(int levelId) const
{
    if (!persistenceManager_) return 0;
    return persistenceManager_->starsForLevel(levelId);
}

bool GameController::canBusMove(int busId) const
{
    if (!currentEngine_ || !isAcceptingInput()) return false;
    return currentEngine_->canBusMove(busId);
}

QString GameController::toString() const
{
    return QString("GameController[state=%1 level=%2 moves=%3 elapsed=%4s score=%5]")
        .arg(static_cast<int>(gameState_))
        .arg(currentLevelId_)
        .arg(moveCount())
        .arg(elapsedSeconds_)
        .arg(currentScore_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots de navegação
// ─────────────────────────────────────────────────────────────────────────────

void GameController::goToMenu()
{
    qDebug().noquote() << "GameController::goToMenu";

    if (gameState_ == GameState::Playing || gameState_ == GameState::Paused)
        saveProgress();

    teardownCurrentLevel();
    transitionTo(GameState::Menu);
}

void GameController::goToLevelSelection()
{
    qDebug().noquote() << "GameController::goToLevelSelection";

    if (gameState_ != GameState::Menu
     && gameState_ != GameState::Won
     && gameState_ != GameState::Lost)
    {
        qWarning().noquote()
            << "GameController::goToLevelSelection: transição inválida de"
            << static_cast<int>(gameState_);
        emit errorOccurred(QStringLiteral("Não é possível ir para a seleção de nível agora."));
        return;
    }

    transitionTo(GameState::LevelSelection);
}

void GameController::loadLevel(int levelId)
{
    qDebug().noquote() << "GameController::loadLevel id=" << levelId;

    if (levelId < 1)
    {
        qWarning().noquote() << "GameController::loadLevel: id inválido (" << levelId << ").";
        emit errorOccurred(QStringLiteral("Id de nível inválido."));
        return;
    }

    if (!isLevelUnlocked(levelId))
    {
        qWarning().noquote()
            << "GameController::loadLevel: nível" << levelId << "bloqueado."
            << "Máximo desbloqueado:" << highestUnlockedLevel_;
        emit errorOccurred(QStringLiteral("Nível ainda não desbloqueado."));
        return;
    }

    teardownCurrentLevel();

    // ── Construção do Board via LevelManager ──────────────────────────────

    if (levelManager_ && levelManager_->hasLevel(levelId))
    {
        LoadResult loadResult;
        currentBoard_ = levelManager_->buildBoardForLevel(levelId, loadResult);

        if (!currentBoard_ || !loadResult.success)
        {
            qWarning().noquote()
                << "GameController::loadLevel: falha ao construir Board para nível"
                << levelId << "—" << loadResult.errorMessage;
            emit errorOccurred(
                QStringLiteral("Erro ao carregar nível %1: %2")
                    .arg(levelId)
                    .arg(loadResult.errorMessage));
            return;
        }

        timeLimitSecs_ = levelManager_->timeLimitForLevel(levelId);

        qDebug().noquote()
            << "GameController: Board do nível" << levelId
            << "carregado via LevelManager." << currentBoard_->toString();
    }
    else
    {
        // Fallback de desenvolvimento: Board vazio mínimo 8×8.
        qWarning().noquote()
            << "GameController::loadLevel: LevelManager indisponível ou nível"
            << levelId << "não encontrado. A usar Board de fallback.";
        currentBoard_  = std::make_unique<Board>(8, 8);
        timeLimitSecs_ = GameControllerConstants::kDefaultTimeLimitSecs;
    }

    // ── Criar Engine ──────────────────────────────────────────────────────

    currentEngine_ = std::make_unique<BoardEngine>(*currentBoard_);
    connectEngineCallbacks();

    // ── Populer os modelos de lista ───────────────────────────────────────

    resetListModels();

    // ── Repor métricas ────────────────────────────────────────────────────

    currentLevelId_ = levelId;
    elapsedSeconds_ = 0;
    currentScore_   = GameControllerConstants::kScoreBasePoints;
    lastResult_     = LevelResult{};

    frameTimer_.start();
    gameTimer_.start();

    transitionTo(GameState::Playing);

    emit currentLevelChanged();
    notifyEngineProperties();
    emit currentScoreChanged();
    emit undoAvailabilityChanged();
    emit hintAvailabilityChanged();

    qDebug().noquote()
        << "GameController: nível" << levelId << "pronto."
        << currentEngine_->toString();
}

void GameController::pauseGame()
{
    if (gameState_ != GameState::Playing)
    {
        qWarning().noquote()
            << "GameController::pauseGame: estado inválido" << static_cast<int>(gameState_);
        return;
    }

    gameTimer_.stop();
    transitionTo(GameState::Paused);

    qDebug().noquote() << "GameController: pausado em" << elapsedSeconds_ << "s.";
}

void GameController::resumeGame()
{
    if (gameState_ != GameState::Paused)
    {
        qWarning().noquote()
            << "GameController::resumeGame: estado inválido" << static_cast<int>(gameState_);
        return;
    }

    frameTimer_.restart();
    gameTimer_.start();
    transitionTo(GameState::Playing);

    qDebug().noquote() << "GameController: retomado.";
}

void GameController::restartLevel()
{
    if (currentLevelId_ < 1)
    {
        qWarning() << "GameController::restartLevel: nenhum nível carregado.";
        return;
    }

    if (gameState_ != GameState::Playing
     && gameState_ != GameState::Paused
     && gameState_ != GameState::Won
     && gameState_ != GameState::Lost)
    {
        qWarning().noquote()
            << "GameController::restartLevel: transição inválida de"
            << static_cast<int>(gameState_);
        return;
    }

    gameTimer_.stop();

    if (currentEngine_)
        currentEngine_->restart();

    // Repopula os modelos a partir do Board reiniciado.
    resetListModels();

    elapsedSeconds_ = 0;
    currentScore_   = GameControllerConstants::kScoreBasePoints;

    frameTimer_.restart();
    gameTimer_.start();

    transitionTo(GameState::Playing);

    notifyEngineProperties();
    emit moveCountChanged();
    emit elapsedSecondsChanged();
    emit remainingSecondsChanged();
    emit currentScoreChanged();
    emit undoAvailabilityChanged();

    qDebug().noquote() << "GameController: nível" << currentLevelId_ << "reiniciado.";
}

void GameController::nextLevel()
{
    if (gameState_ != GameState::Won)
    {
        qWarning().noquote() << "GameController::nextLevel: só válido em Won.";
        emit errorOccurred(QStringLiteral("Conclua o nível antes de avançar."));
        return;
    }

    int nextId = currentLevelId_ + 1;  // Sequência por omissão.

    if (levelManager_)
    {
        const int managed = levelManager_->nextLevelId(currentLevelId_);
        if (managed != LevelManagerConstants::kNoNextLevel)
            nextId = managed;
        else
        {
            qDebug() << "GameController::nextLevel: sem mais níveis disponíveis.";
            emit errorOccurred(QStringLiteral("Não existem mais níveis disponíveis."));
            goToLevelSelection();
            return;
        }
    }

    qDebug().noquote() << "GameController::nextLevel: avançando para" << nextId;
    loadLevel(nextId);
}

// ─────────────────────────────────────────────────────────────────────────────
// Input do jogador
// ─────────────────────────────────────────────────────────────────────────────

void GameController::onBusTapped(int busId)
{
    if (!isAcceptingInput())
    {
        qDebug().noquote()
            << "GameController::onBusTapped: input ignorado (estado="
            << static_cast<int>(gameState_) << ")";
        emit busTapped(busId, MoveOutcome::NotPlaying);
        return;
    }

    if (!currentEngine_)
    {
        qWarning() << "GameController::onBusTapped: engine nulo.";
        emit busTapped(busId, MoveOutcome::NotPlaying);
        return;
    }

    qDebug().noquote() << "GameController: toque em Bus id=" << busId;

    const MoveResult  engineResult = currentEngine_->requestBusMove(busId);
    const MoveOutcome outcome      = toMoveOutcome(engineResult);

    if (engineResult == MoveResult::Success)
    {
        notifyEngineProperties();
        updateScore();
    }

    emit busTapped(busId, outcome);
}

void GameController::requestUndo()
{
    if (!isAcceptingInput() || !currentEngine_)
    {
        qWarning() << "GameController::requestUndo: indisponível.";
        return;
    }

    if (!currentEngine_->canUndo())
    {
        qDebug() << "GameController::requestUndo: pilha vazia.";
        return;
    }

    const bool undone = currentEngine_->undoLastMove();

    if (undone)
    {
        resetListModels();   // Reconstrói modelos a partir do estado revertido.
        notifyEngineProperties();
        updateScore();
        emit undoAvailabilityChanged();
        qDebug() << "GameController: undo executado.";
    }
}

void GameController::requestHint()
{
    if (!isAcceptingInput() || hintsRemaining_ <= 0)
    {
        qDebug() << "GameController::requestHint: sem hints disponíveis.";
        return;
    }

    --hintsRemaining_;
    emit hintRequested();
    emit hintAvailabilityChanged();

    qDebug().noquote()
        << "GameController: hint solicitado. Restantes:" << hintsRemaining_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Slot interno: timer
// ─────────────────────────────────────────────────────────────────────────────

void GameController::onTimerTick()
{
    if (gameState_ != GameState::Playing) return;

    ++elapsedSeconds_;

    emit elapsedSecondsChanged();
    emit remainingSecondsChanged();
    emit timerTick(elapsedSeconds_, remainingSeconds());

    updateScore();

    if (timeLimitSecs_ > 0 && elapsedSeconds_ >= timeLimitSecs_)
    {
        qDebug().noquote()
            << "GameController: tempo esgotado após" << elapsedSeconds_ << "s.";
        handleLevelLost();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Métodos privados
// ─────────────────────────────────────────────────────────────────────────────

void GameController::transitionTo(GameState newState)
{
    if (gameState_ == newState) return;

    qDebug().noquote()
        << "GameController:"
        << static_cast<int>(gameState_) << "→" << static_cast<int>(newState);

    gameState_ = newState;
    emit gameStateChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// connectEngineCallbacks — núcleo da integração updateX()
// ─────────────────────────────────────────────────────────────────────────────

void GameController::connectEngineCallbacks()
{
    if (!currentEngine_) return;

    // ── Callback: Bus moveu-se uma célula ─────────────────────────────────
    //
    // O Bus pode ainda estar a caminho da plataforma (múltiplos passos) ou
    // acabar de chegar. Actualizamos a posição no modelo e emitimos o sinal
    // para a animação QML.
    currentEngine_->setOnBusMoved(
        [this](int busId, QPoint from, QPoint to)
        {
            // Consulta o estado actualizado directamente no Board.
            const Bus* bus = currentBoard_ ? currentBoard_->findBusById(busId) : nullptr;

            if (bus)
            {
                busListModel_->updateBus(
                    busId,
                    bus->row(),
                    bus->col(),
                    bus->boardedCount(),
                    bus->isAtPlatform(),
                    bus->hasLeft()
                );
            }

            // Sinal para a animação QML (BusItem usa SmoothedAnimation em x/y).
            emit busMoved(busId, from.y(), from.x(), to.y(), to.x());
        }
    );

    // ── Callback: passageiro embarcou ─────────────────────────────────────
    //
    // Sequência em dois tempos:
    //  1. Imediato: marca como Boarding → a animação de encolher começa.
    //  2. Após kBoardingAnimMs: marca como Boarded → PassengerItem desaparece.
    // Também actualiza a contagem de embarcados no Bus.
    currentEngine_->setOnPassengerBoarded(
        [this](int passengerId, int busId)
        {
            // Passo 1 — Boarding (inicia animação no QML).
            passengerListModel_->updatePassenger(passengerId,
                                                  /*isBoarding=*/ true,
                                                  /*isBoarded=*/  false);

            // Actualiza a barra de lotação do autocarro.
            const Bus* bus = currentBoard_ ? currentBoard_->findBusById(busId) : nullptr;
            if (bus)
            {
                busListModel_->updateBus(
                    busId,
                    bus->row(),
                    bus->col(),
                    bus->boardedCount(),
                    bus->isAtPlatform(),
                    bus->hasLeft()
                );
            }

            // Sinal para o GameScreen (som, vibração, etc.).
            emit passengerBoarded(passengerId, busId);

            // Passo 2 — Boarded (após animação de boarding).
            QTimer::singleShot(
                GameControllerConstants::kBoardingAnimMs,
                this,
                [this, passengerId]()
                {
                    passengerListModel_->updatePassenger(passengerId,
                                                          /*isBoarding=*/ false,
                                                          /*isBoarded=*/  true);
                }
            );
        }
    );

    // ── Callback: autocarro saiu do tabuleiro ─────────────────────────────
    //
    // Marca o Bus como hasLeft no modelo; a plataforma fica Completed.
    currentEngine_->setOnBusLeft(
        [this](int busId)
        {
            // Bus: posição inválida, hasLeft=true.
            busListModel_->updateBus(
                busId,
                /*row=*/        0,
                /*col=*/        0,
                /*boardedCount=*/ 0,
                /*isAtPlatform=*/ false,
                /*hasLeft=*/      true
            );

            // Plataforma: encontrar a plataforma que estava Occupied por este Bus
            // e marcá-la como Completed.
            if (currentBoard_)
            {
                for (const auto& platPtr : currentBoard_->platforms())
                {
                    const Platform* pl = platPtr.get();
                    // A plataforma já foi concluída pelo Engine antes deste callback;
                    // consultamos o estado real do Board.
                    platformListModel_->updatePlatform(
                        pl->id(),
                        static_cast<int>(pl->state())
                    );
                }
            }

            emit busLeft(busId);
        }
    );

    // ── Callback: nível concluído ─────────────────────────────────────────

    currentEngine_->setOnLevelComplete(
        [this]()
        {
            handleLevelWon();
        }
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// resetListModels — popula os três modelos a partir do Board
// ─────────────────────────────────────────────────────────────────────────────

void GameController::resetListModels()
{
    if (!currentBoard_)
    {
        busListModel_->clear();
        passengerListModel_->clear();
        platformListModel_->clear();
        return;
    }

    busListModel_->resetFromBoard(*currentBoard_);
    passengerListModel_->resetFromBoard(*currentBoard_);
    platformListModel_->resetFromBoard(*currentBoard_);

    qDebug().noquote()
        << "GameController: modelos de lista populados."
        << "buses=" << busListModel_->rowCount()
        << "passengers=" << passengerListModel_->rowCount()
        << "platforms=" << platformListModel_->rowCount();
}

// ─────────────────────────────────────────────────────────────────────────────

void GameController::teardownCurrentLevel()
{
    gameTimer_.stop();

    // Limpar modelos antes de destruir os objectos que eles referenciam.
    busListModel_->clear();
    passengerListModel_->clear();
    platformListModel_->clear();

    // Engine antes do Board (Engine guarda referência ao Board).
    currentEngine_.reset();
    currentBoard_.reset();

    currentLevelId_ = -1;
    elapsedSeconds_ = 0;
    timeLimitSecs_  = GameControllerConstants::kDefaultTimeLimitSecs;
    currentScore_   = 0;
}

void GameController::updateScore()
{
    if (!currentEngine_) return;

    const int moves   = currentEngine_->moveCount();
    const int elapsed = elapsedSeconds_;

    int newScore = GameControllerConstants::kScoreBasePoints
                 - (moves   * GameControllerConstants::kScorePerMovePenalty)
                 - (elapsed * GameControllerConstants::kScorePerSecPenalty);

    newScore = std::max(0, newScore);

    if (newScore != currentScore_)
    {
        currentScore_ = newScore;
        emit currentScoreChanged();
    }
}

int GameController::calcStars(int moves, int elapsedSecs) noexcept
{
    Q_UNUSED(elapsedSecs)

    if (moves <= GameControllerConstants::kStar3MaxMoves) return 3;
    if (moves <= GameControllerConstants::kStar2MaxMoves) return 2;
    return 1;
}

LevelResult GameController::buildLevelResult(bool completed) const
{
    const int moves   = currentEngine_ ? currentEngine_->moveCount() : 0;
    const int elapsed = elapsedSeconds_;

    LevelResult r;
    r.levelId     = currentLevelId_;
    r.completed   = completed;
    r.moves       = moves;
    r.elapsedSecs = elapsed;
    r.score       = completed ? currentScore_ : 0;
    r.stars       = completed ? calcStars(moves, elapsed) : 0;
    return r;
}

void GameController::handleLevelWon()
{
    if (gameState_ == GameState::Won) return;

    gameTimer_.stop();

    lastResult_ = buildLevelResult(true);

    if (lastResult_.levelId >= highestUnlockedLevel_)
    {
        highestUnlockedLevel_ = lastResult_.levelId + 1;
        emit progressChanged();
    }

    saveProgress();
    transitionTo(GameState::Won);
    emit levelWon(lastResult_);

    qDebug().noquote()
        << "GameController: VITÓRIA! Nível" << currentLevelId_
        << "score=" << lastResult_.score
        << "estrelas=" << lastResult_.stars
        << "jogadas=" << lastResult_.moves
        << "tempo=" << lastResult_.elapsedSecs << "s";
}

void GameController::handleLevelLost()
{
    if (gameState_ == GameState::Lost) return;

    gameTimer_.stop();

    lastResult_ = buildLevelResult(false);

    transitionTo(GameState::Lost);
    emit levelLost(lastResult_);

    qDebug().noquote()
        << "GameController: DERROTA. Nível" << currentLevelId_
        << "tempo=" << lastResult_.elapsedSecs << "s";
}

void GameController::saveProgress()
{
    if (!persistenceManager_)
    {
        qDebug() << "GameController::saveProgress: PersistenceManager indisponível.";
        return;
    }

    persistenceManager_->saveHighestUnlockedLevel(highestUnlockedLevel_);
    persistenceManager_->saveLastPlayedLevel(currentLevelId_);

    if (lastResult_.completed)
    {
        persistenceManager_->saveLevelResult(
            lastResult_.levelId,
            lastResult_.score,
            lastResult_.moves,
            lastResult_.elapsedSecs,
            lastResult_.stars
        );
    }

    persistenceManager_->flush();
    emit progressSaved();

    qDebug().noquote()
        << "GameController: progresso guardado."
        << "highestLevel=" << highestUnlockedLevel_;
}

void GameController::loadProgress()
{
    if (!persistenceManager_)
    {
        qDebug() << "GameController::loadProgress: PersistenceManager indisponível.";
        return;
    }

    highestUnlockedLevel_ = persistenceManager_->loadHighestUnlockedLevel();

    qDebug().noquote()
        << "GameController: progresso carregado."
        << "highestLevel=" << highestUnlockedLevel_;
}

MoveOutcome GameController::toMoveOutcome(MoveResult result) noexcept
{
    switch (result)
    {
        case MoveResult::Success:      return MoveOutcome::Success;
        case MoveResult::Blocked:      return MoveOutcome::Blocked;
        case MoveResult::AlreadyLeft:  return MoveOutcome::AlreadyLeft;
        case MoveResult::BusNotFound:  return MoveOutcome::BusNotFound;
        case MoveResult::NoPlatform:   return MoveOutcome::NoPlatform;
        case MoveResult::InvalidState: return MoveOutcome::NotPlaying;
    }
    Q_UNREACHABLE();
    return MoveOutcome::NotPlaying;
}

void GameController::notifyEngineProperties()
{
    emit remainingPassengersChanged();
    emit remainingBusesChanged();
    emit moveCountChanged();
    emit undoAvailabilityChanged();
}

bool GameController::isAcceptingInput() const noexcept
{
    return gameState_ == GameState::Playing;
}
