/**
 * @file GameController.h
 * @brief Declaração da classe GameController para o jogo Bus Jam Puzzle.
 *
 * GameController é o ponto central de coordenação do jogo. Herda de QObject
 * para suportar sinais/slots e Q_PROPERTY, tornando-o directamente exposto
 * ao motor QML via setContextProperty.
 *
 * ### Posição na arquitectura:
 *
 *  QML / UI
 *     │  (Q_PROPERTY, signals, slots)
 *  GameController
 *     ├── BoardEngine
 *     ├── Board
 *     ├── BusListModel        ← novo: ponte C++ → QML para autocarros
 *     ├── PassengerListModel  ← novo: ponte C++ → QML para passageiros
 *     ├── PlatformListModel   ← novo: ponte C++ → QML para plataformas
 *     ├── LevelManager
 *     └── PersistenceManager
 *
 * ### Fluxo de actualização dos modelos:
 *
 *  onBusTapped(id)
 *    → BoardEngine::requestBusMove()
 *        → onBusMoved_       → busListModel_->updateBus(row, col, …)
 *        → onPassengerBoarded_ → passengerListModel_->updatePassenger(boarding=true)
 *                              → QTimer::singleShot(300ms) → updatePassenger(boarded=true)
 *        → onBusLeft_        → busListModel_->updateBus(hasLeft=true)
 *        → onPlatformChanged_→ platformListModel_->updatePlatform(stateEnum)
 *        → onLevelComplete_  → handleLevelWon()
 *
 * ### Thread-safety:
 *  Não thread-safe. Todas as chamadas na UI thread.
 */

#pragma once

#include "BoardEngine.h"
#include "BusListModel.h"
#include "PassengerListModel.h"
#include "PlatformListModel.h"

#include <QObject>
#include <QAbstractListModel>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>
#include <memory>

class LevelManager;
class PersistenceManager;

// ─────────────────────────────────────────────────────────────────────────────
// Enumerações expostas ao QML
// ─────────────────────────────────────────────────────────────────────────────

enum class GameState : quint8
{
    Menu           = 0,
    LevelSelection = 1,
    Playing        = 2,
    Paused         = 3,
    Won            = 4,
    Lost           = 5
};
Q_DECLARE_METATYPE(GameState)

enum class MoveOutcome : quint8
{
    Success      = 0,
    Blocked      = 1,
    AlreadyLeft  = 2,
    BusNotFound  = 3,
    NoPlatform   = 4,
    NotPlaying   = 5
};
Q_DECLARE_METATYPE(MoveOutcome)

// ─────────────────────────────────────────────────────────────────────────────
// Estrutura de resultado de nível
// ─────────────────────────────────────────────────────────────────────────────

struct LevelResult
{
    int  levelId      { -1 };
    bool completed    { false };
    int  moves        { 0 };
    int  elapsedSecs  { 0 };
    int  score        { 0 };
    int  stars        { 0 };
};

// ─────────────────────────────────────────────────────────────────────────────
// Constantes
// ─────────────────────────────────────────────────────────────────────────────

namespace GameControllerConstants
{
    constexpr int kTimerIntervalMs      = 1000;
    constexpr int kDefaultTimeLimitSecs = 0;
    constexpr int kScoreBasePoints      = 1000;
    constexpr int kScorePerMovePenalty  = 10;
    constexpr int kScorePerSecPenalty   = 2;
    constexpr int kStar3MaxMoves        = 5;
    constexpr int kStar2MaxMoves        = 10;

    /// Duração (ms) da animação de boarding antes de marcar Passenger como Boarded.
    /// Deve ser >= à duração da animação PassengerItem em QML.
    constexpr int kBoardingAnimMs = 300;
}

// ─────────────────────────────────────────────────────────────────────────────
// Classe GameController
// ─────────────────────────────────────────────────────────────────────────────

class GameController : public QObject
{
    Q_OBJECT

    // ─── Q_PROPERTY: estado e métricas ────────────────────────────────────

    Q_PROPERTY(GameState gameState
               READ  gameState
               NOTIFY gameStateChanged)

    Q_PROPERTY(int currentLevelId
               READ  currentLevelId
               NOTIFY currentLevelChanged)

    Q_PROPERTY(int moveCount
               READ  moveCount
               NOTIFY moveCountChanged)

    Q_PROPERTY(int elapsedSeconds
               READ  elapsedSeconds
               NOTIFY elapsedSecondsChanged)

    Q_PROPERTY(int remainingSeconds
               READ  remainingSeconds
               NOTIFY remainingSecondsChanged)

    Q_PROPERTY(int remainingPassengers
               READ  remainingPassengers
               NOTIFY remainingPassengersChanged)

    Q_PROPERTY(int remainingBuses
               READ  remainingBuses
               NOTIFY remainingBusesChanged)

    Q_PROPERTY(int currentScore
               READ  currentScore
               NOTIFY currentScoreChanged)

    Q_PROPERTY(int highestUnlockedLevel
               READ  highestUnlockedLevel
               NOTIFY progressChanged)

    Q_PROPERTY(bool canUndo
               READ  canUndo
               NOTIFY undoAvailabilityChanged)

    Q_PROPERTY(bool canHint
               READ  canHint
               NOTIFY hintAvailabilityChanged)

    // ─── Q_PROPERTY: modelos de lista (BoardView.qml) ─────────────────────

    /**
     * @brief Modelo de autocarros, directamente usável como Repeater.model.
     *
     * Roles: busId, colorIndex, capacity, boardedCount, boardRow, boardCol,
     *        directionEnum, isActive, isAtPlatform, hasLeft.
     *
     * CONSTANT: o ponteiro nunca muda; apenas os dados internos mudam
     * via dataChanged(), que o Repeater detecta automaticamente.
     */
    Q_PROPERTY(QAbstractListModel* busModel
               READ  busModel
               CONSTANT)

    /**
     * @brief Modelo de passageiros.
     *
     * Roles: passengerId, colorIndex, cellRow, cellCol,
     *        stateEnum, isWaiting, isBoarding, isBoarded.
     */
    Q_PROPERTY(QAbstractListModel* passengerModel
               READ  passengerModel
               CONSTANT)

    /**
     * @brief Modelo de plataformas.
     *
     * Roles: platformId, colorIndex, platformRow, platformCol,
     *        exitDirEnum, stateEnum, isFree, isOccupied, isCompleted.
     */
    Q_PROPERTY(QAbstractListModel* platformModel
               READ  platformModel
               CONSTANT)

    /**
     * @brief Dimensões do tabuleiro actual (actualizado com currentLevelChanged).
     */
    Q_PROPERTY(int boardRows
               READ  boardRows
               NOTIFY currentLevelChanged)

    Q_PROPERTY(int boardCols
               READ  boardCols
               NOTIFY currentLevelChanged)

public:
    // ─── Construção e destruição ───────────────────────────────────────────

    explicit GameController(LevelManager*       levelManager,
                            PersistenceManager* persistenceManager,
                            QObject*            parent = nullptr);

    ~GameController() override;

    // ─── Leitores de Q_PROPERTY ────────────────────────────────────────────

    [[nodiscard]] GameState gameState()            const noexcept;
    [[nodiscard]] int       currentLevelId()       const noexcept;
    [[nodiscard]] int       moveCount()            const noexcept;
    [[nodiscard]] int       elapsedSeconds()       const noexcept;
    [[nodiscard]] int       remainingSeconds()     const noexcept;
    [[nodiscard]] int       remainingPassengers()  const noexcept;
    [[nodiscard]] int       remainingBuses()       const noexcept;
    [[nodiscard]] int       currentScore()         const noexcept;
    [[nodiscard]] int       highestUnlockedLevel() const noexcept;
    [[nodiscard]] bool      canUndo()              const noexcept;
    [[nodiscard]] bool      canHint()              const noexcept;

    // Modelos de lista — ponteiros CONSTANT (filhos QObject, possuídos)
    [[nodiscard]] QAbstractListModel* busModel()       const noexcept;
    [[nodiscard]] QAbstractListModel* passengerModel() const noexcept;
    [[nodiscard]] QAbstractListModel* platformModel()  const noexcept;

    // Dimensões do tabuleiro
    [[nodiscard]] int boardRows() const noexcept;
    [[nodiscard]] int boardCols() const noexcept;

    // Outros getters
    [[nodiscard]] const BoardEngine* engine()           const noexcept;
    [[nodiscard]] const LevelResult& lastResult()       const noexcept;
    [[nodiscard]] bool isLevelUnlocked(int levelId)     const noexcept;

    // ─── Q_INVOKABLE ───────────────────────────────────────────────────────

    /**
     * @brief Passageiros embarcados num autocarro específico.
     *
     * Usado pelo BoardView.qml para actualizar a barra de lotação
     * quando o modelo ainda não reflecte a última jogada.
     */
    Q_INVOKABLE int boardedCountFor(int busId) const;

    /**
     * @brief Estrelas obtidas num nível (0-3). 0 = nunca completado.
     *
     * Usado pelo LevelSelectScreen.qml para mostrar progresso.
     */
    Q_INVOKABLE int starsForLevel(int levelId) const;

    /**
     * @brief Indica se um autocarro pode mover-se agora.
     *
     * Usado pelo BoardView.qml para o highlight de hint.
     */
    Q_INVOKABLE bool canBusMove(int busId) const;

    [[nodiscard]] QString toString() const;

public slots:
    // ─── Navegação ─────────────────────────────────────────────────────────

    void goToMenu();
    void goToLevelSelection();
    void loadLevel(int levelId);
    void pauseGame();
    void resumeGame();
    void restartLevel();
    void nextLevel();

    // ─── Input do jogador ──────────────────────────────────────────────────

    void onBusTapped(int busId);
    void requestUndo();
    void requestHint();

signals:
    // Q_PROPERTY notifications
    void gameStateChanged();
    void currentLevelChanged();
    void moveCountChanged();
    void elapsedSecondsChanged();
    void remainingSecondsChanged();
    void remainingPassengersChanged();
    void remainingBusesChanged();
    void currentScoreChanged();
    void progressChanged();
    void undoAvailabilityChanged();
    void hintAvailabilityChanged();

    // Eventos de jogo (para animações no QML)
    void busTapped(int busId, MoveOutcome outcome);
    void busMoved(int busId, int fromRow, int fromCol, int toRow, int toCol);
    void passengerBoarded(int passengerId, int busId);
    void busLeft(int busId);
    void levelWon(LevelResult result);
    void levelLost(LevelResult result);
    void timerTick(int elapsed, int remaining);
    void hintRequested();
    void progressSaved();
    void errorOccurred(QString message);

private slots:
    void onTimerTick();

private:
    // ─── Dependências injectadas (não possui) ─────────────────────────────

    LevelManager*       levelManager_;
    PersistenceManager* persistenceManager_;

    // ─── Modelos de lista (possuídos como filhos QObject) ─────────────────

    BusListModel*       busListModel_;        ///< Criado no construtor com this como pai.
    PassengerListModel* passengerListModel_;
    PlatformListModel*  platformListModel_;

    // ─── Estado global ────────────────────────────────────────────────────

    GameState gameState_;
    int       currentLevelId_;
    int       highestUnlockedLevel_;
    int       hintsRemaining_;

    // ─── Objectos do nível actual (possuídos via unique_ptr) ──────────────

    std::unique_ptr<Board>       currentBoard_;
    std::unique_ptr<BoardEngine> currentEngine_;

    // ─── Métricas do nível ────────────────────────────────────────────────

    int  elapsedSeconds_;
    int  timeLimitSecs_;
    int  currentScore_;

    LevelResult lastResult_;

    // ─── Timer ────────────────────────────────────────────────────────────

    QTimer        gameTimer_;
    QElapsedTimer frameTimer_;

    // ─── Métodos privados ─────────────────────────────────────────────────

    void transitionTo(GameState newState);

    /**
     * @brief Liga todos os callbacks do BoardEngine aos modelos e sinais.
     *
     * Chamado após criar currentEngine_ em loadLevel() e restartLevel().
     * Cada callback actualiza o modelo de lista correspondente E emite
     * o sinal Qt para o QML reagir com animações.
     */
    void connectEngineCallbacks();

    /**
     * @brief Popula os três modelos de lista a partir do Board actual.
     *
     * Chamado após criar/resetar o Board, antes de emitir currentLevelChanged.
     */
    void resetListModels();

    /**
     * @brief Limpa os modelos e destrói o Board/Engine actuais.
     */
    void teardownCurrentLevel();

    void updateScore();
    void handleLevelWon();
    void handleLevelLost();
    void saveProgress();
    void loadProgress();
    void notifyEngineProperties();

    [[nodiscard]] bool isAcceptingInput()                           const noexcept;
    [[nodiscard]] LevelResult buildLevelResult(bool completed)      const;
    [[nodiscard]] static int calcStars(int moves, int secs)         noexcept;
    [[nodiscard]] static MoveOutcome toMoveOutcome(MoveResult r)    noexcept;
};
