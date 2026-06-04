/**
 * @file Enums.h
 * @brief Enumerações partilhadas do domínio Bus Jam Puzzle.
 *
 * Centraliza BusColor, Direction, PassengerState, PlatformState,
 * GameState e MoveOutcome num único header sem dependências de Qt.
 *
 * ### Registo em QML:
 *  No main.cpp, após criar o engine:
 *
 *    qmlRegisterUncreatableType<BusJam::Enums>(
 *        "BusJam", 1, 0, "BusJam",
 *        "BusJam é apenas um namespace de enums");
 *
 *  Em QML:
 *    import BusJam 1.0
 *    ...
 *    color: busColorToHex(BusJam.Red)
 *    if (gameController.gameState === BusJam.Playing) { ... }
 *
 * ### Compatibilidade com os headers existentes:
 *  Bus.h, Passenger.h, Platform.h e GameController.h declaram os seus
 *  próprios enums localmente. Após integração completa, esses headers
 *  devem incluir Enums.h e usar os tipos aqui declarados.
 *  Por agora, este ficheiro é autónomo e serve o main.cpp + QML.
 */

#pragma once

#include <QObject>
#include <QtGlobal>

// ─────────────────────────────────────────────────────────────────────────────
// Namespace / classe de registo QML
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Classe contentor para todos os enums expostos ao QML.
 *
 * Herda de QObject para suportar Q_ENUM e qmlRegisterUncreatableType.
 * Não é instanciável directamente.
 */
class BusJamEnums : public QObject
{
    Q_OBJECT

public:
    // Construtor explicitamente deletado — não instanciável.
    BusJamEnums() = delete;

    // ─── BusColor ─────────────────────────────────────────────────────────

    /**
     * @brief Cores disponíveis para autocarros e passageiros.
     */
    enum BusColor : quint8
    {
        Red    = 0,
        Blue   = 1,
        Green  = 2,
        Yellow = 3,
        Purple = 4
    };
    Q_ENUM(BusColor)

    // ─── Direction ────────────────────────────────────────────────────────

    /**
     * @brief Direcção de movimento de um autocarro.
     */
    enum Direction : quint8
    {
        Up    = 0,
        Down  = 1,
        Left  = 2,
        Right = 3
    };
    Q_ENUM(Direction)

    // ─── PassengerState ───────────────────────────────────────────────────

    /**
     * @brief Estado do ciclo de vida de um passageiro.
     */
    enum PassengerState : quint8
    {
        Waiting  = 0,
        Boarding = 1,
        Boarded  = 2
    };
    Q_ENUM(PassengerState)

    // ─── PlatformState ────────────────────────────────────────────────────

    /**
     * @brief Estado de uma plataforma de embarque.
     */
    enum PlatformState : quint8
    {
        Free      = 0,
        Occupied  = 1,
        Completed = 2
    };
    Q_ENUM(PlatformState)

    // ─── GameState ────────────────────────────────────────────────────────

    /**
     * @brief Estados da máquina de estados principal do jogo.
     */
    enum GameState : quint8
    {
        Menu           = 0,
        LevelSelection = 1,
        Playing        = 2,
        Paused         = 3,
        Won            = 4,
        Lost           = 5
    };
    Q_ENUM(GameState)

    // ─── MoveOutcome ──────────────────────────────────────────────────────

    /**
     * @brief Resultado de um pedido de movimento de autocarro.
     */
    enum MoveOutcome : quint8
    {
        MoveSuccess     = 0,
        MoveBlocked     = 1,
        MoveAlreadyLeft = 2,
        MoveBusNotFound = 3,
        MoveNoPlatform  = 4,
        MoveNotPlaying  = 5
    };
    Q_ENUM(MoveOutcome)
};

// ─────────────────────────────────────────────────────────────────────────────
// Aliases globais (para compatibilidade com o código C++ existente)
// ─────────────────────────────────────────────────────────────────────────────
// Os headers existentes (Bus.h, etc.) declaram os seus próprios enums
// com os mesmos nomes. Quando forem migrados para usar Enums.h, os
// aliases abaixo garantem compatibilidade sem alterar o código existente.

// using BusColor      = BusJamEnums::BusColor;
// using Direction     = BusJamEnums::Direction;
// using PassengerState = BusJamEnums::PassengerState;
// using PlatformState = BusJamEnums::PlatformState;
// using GameState     = BusJamEnums::GameState;
// using MoveOutcome   = BusJamEnums::MoveOutcome;

// Descomentados após migração completa dos headers existentes.
