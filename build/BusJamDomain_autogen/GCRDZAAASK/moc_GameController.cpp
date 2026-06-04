/****************************************************************************
** Meta object code from reading C++ file 'GameController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/engine/GameController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14GameControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto GameController::qt_create_metaobjectdata<qt_meta_tag_ZN14GameControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GameController",
        "gameStateChanged",
        "",
        "currentLevelChanged",
        "moveCountChanged",
        "elapsedSecondsChanged",
        "remainingSecondsChanged",
        "remainingPassengersChanged",
        "remainingBusesChanged",
        "currentScoreChanged",
        "progressChanged",
        "undoAvailabilityChanged",
        "hintAvailabilityChanged",
        "busTapped",
        "busId",
        "MoveOutcome",
        "outcome",
        "busMoved",
        "fromRow",
        "fromCol",
        "toRow",
        "toCol",
        "passengerBoarded",
        "passengerId",
        "busLeft",
        "levelWon",
        "LevelResult",
        "result",
        "levelLost",
        "timerTick",
        "elapsed",
        "remaining",
        "hintRequested",
        "progressSaved",
        "errorOccurred",
        "message",
        "goToMenu",
        "goToLevelSelection",
        "loadLevel",
        "levelId",
        "pauseGame",
        "resumeGame",
        "restartLevel",
        "nextLevel",
        "onBusTapped",
        "requestUndo",
        "requestHint",
        "onTimerTick",
        "boardedCountFor",
        "starsForLevel",
        "canBusMove",
        "gameState",
        "GameState",
        "currentLevelId",
        "moveCount",
        "elapsedSeconds",
        "remainingSeconds",
        "remainingPassengers",
        "remainingBuses",
        "currentScore",
        "highestUnlockedLevel",
        "canUndo",
        "canHint",
        "busModel",
        "QAbstractListModel*",
        "passengerModel",
        "platformModel",
        "boardRows",
        "boardCols"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'gameStateChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentLevelChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'moveCountChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'elapsedSecondsChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'remainingSecondsChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'remainingPassengersChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'remainingBusesChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'currentScoreChanged'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'progressChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'undoAvailabilityChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'hintAvailabilityChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'busTapped'
        QtMocHelpers::SignalData<void(int, MoveOutcome)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { 0x80000000 | 15, 16 },
        }}),
        // Signal 'busMoved'
        QtMocHelpers::SignalData<void(int, int, int, int, int)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { QMetaType::Int, 18 }, { QMetaType::Int, 19 }, { QMetaType::Int, 20 },
            { QMetaType::Int, 21 },
        }}),
        // Signal 'passengerBoarded'
        QtMocHelpers::SignalData<void(int, int)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 23 }, { QMetaType::Int, 14 },
        }}),
        // Signal 'busLeft'
        QtMocHelpers::SignalData<void(int)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 },
        }}),
        // Signal 'levelWon'
        QtMocHelpers::SignalData<void(LevelResult)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 26, 27 },
        }}),
        // Signal 'levelLost'
        QtMocHelpers::SignalData<void(LevelResult)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 26, 27 },
        }}),
        // Signal 'timerTick'
        QtMocHelpers::SignalData<void(int, int)>(29, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 30 }, { QMetaType::Int, 31 },
        }}),
        // Signal 'hintRequested'
        QtMocHelpers::SignalData<void()>(32, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'progressSaved'
        QtMocHelpers::SignalData<void()>(33, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(QString)>(34, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 35 },
        }}),
        // Slot 'goToMenu'
        QtMocHelpers::SlotData<void()>(36, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'goToLevelSelection'
        QtMocHelpers::SlotData<void()>(37, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'loadLevel'
        QtMocHelpers::SlotData<void(int)>(38, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 39 },
        }}),
        // Slot 'pauseGame'
        QtMocHelpers::SlotData<void()>(40, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'resumeGame'
        QtMocHelpers::SlotData<void()>(41, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'restartLevel'
        QtMocHelpers::SlotData<void()>(42, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'nextLevel'
        QtMocHelpers::SlotData<void()>(43, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onBusTapped'
        QtMocHelpers::SlotData<void(int)>(44, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 },
        }}),
        // Slot 'requestUndo'
        QtMocHelpers::SlotData<void()>(45, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'requestHint'
        QtMocHelpers::SlotData<void()>(46, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onTimerTick'
        QtMocHelpers::SlotData<void()>(47, 2, QMC::AccessPrivate, QMetaType::Void),
        // Method 'boardedCountFor'
        QtMocHelpers::MethodData<int(int) const>(48, 2, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::Int, 14 },
        }}),
        // Method 'starsForLevel'
        QtMocHelpers::MethodData<int(int) const>(49, 2, QMC::AccessPublic, QMetaType::Int, {{
            { QMetaType::Int, 39 },
        }}),
        // Method 'canBusMove'
        QtMocHelpers::MethodData<bool(int) const>(50, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::Int, 14 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'gameState'
        QtMocHelpers::PropertyData<GameState>(51, 0x80000000 | 52, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'currentLevelId'
        QtMocHelpers::PropertyData<int>(53, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'moveCount'
        QtMocHelpers::PropertyData<int>(54, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'elapsedSeconds'
        QtMocHelpers::PropertyData<int>(55, QMetaType::Int, QMC::DefaultPropertyFlags, 3),
        // property 'remainingSeconds'
        QtMocHelpers::PropertyData<int>(56, QMetaType::Int, QMC::DefaultPropertyFlags, 4),
        // property 'remainingPassengers'
        QtMocHelpers::PropertyData<int>(57, QMetaType::Int, QMC::DefaultPropertyFlags, 5),
        // property 'remainingBuses'
        QtMocHelpers::PropertyData<int>(58, QMetaType::Int, QMC::DefaultPropertyFlags, 6),
        // property 'currentScore'
        QtMocHelpers::PropertyData<int>(59, QMetaType::Int, QMC::DefaultPropertyFlags, 7),
        // property 'highestUnlockedLevel'
        QtMocHelpers::PropertyData<int>(60, QMetaType::Int, QMC::DefaultPropertyFlags, 8),
        // property 'canUndo'
        QtMocHelpers::PropertyData<bool>(61, QMetaType::Bool, QMC::DefaultPropertyFlags, 9),
        // property 'canHint'
        QtMocHelpers::PropertyData<bool>(62, QMetaType::Bool, QMC::DefaultPropertyFlags, 10),
        // property 'busModel'
        QtMocHelpers::PropertyData<QAbstractListModel*>(63, 0x80000000 | 64, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'passengerModel'
        QtMocHelpers::PropertyData<QAbstractListModel*>(65, 0x80000000 | 64, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'platformModel'
        QtMocHelpers::PropertyData<QAbstractListModel*>(66, 0x80000000 | 64, QMC::DefaultPropertyFlags | QMC::EnumOrFlag | QMC::Constant),
        // property 'boardRows'
        QtMocHelpers::PropertyData<int>(67, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'boardCols'
        QtMocHelpers::PropertyData<int>(68, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<GameController, qt_meta_tag_ZN14GameControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GameController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14GameControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14GameControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14GameControllerE_t>.metaTypes,
    nullptr
} };

void GameController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GameController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->gameStateChanged(); break;
        case 1: _t->currentLevelChanged(); break;
        case 2: _t->moveCountChanged(); break;
        case 3: _t->elapsedSecondsChanged(); break;
        case 4: _t->remainingSecondsChanged(); break;
        case 5: _t->remainingPassengersChanged(); break;
        case 6: _t->remainingBusesChanged(); break;
        case 7: _t->currentScoreChanged(); break;
        case 8: _t->progressChanged(); break;
        case 9: _t->undoAvailabilityChanged(); break;
        case 10: _t->hintAvailabilityChanged(); break;
        case 11: _t->busTapped((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<MoveOutcome>>(_a[2]))); break;
        case 12: _t->busMoved((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[5]))); break;
        case 13: _t->passengerBoarded((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->busLeft((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->levelWon((*reinterpret_cast<std::add_pointer_t<LevelResult>>(_a[1]))); break;
        case 16: _t->levelLost((*reinterpret_cast<std::add_pointer_t<LevelResult>>(_a[1]))); break;
        case 17: _t->timerTick((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 18: _t->hintRequested(); break;
        case 19: _t->progressSaved(); break;
        case 20: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->goToMenu(); break;
        case 22: _t->goToLevelSelection(); break;
        case 23: _t->loadLevel((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 24: _t->pauseGame(); break;
        case 25: _t->resumeGame(); break;
        case 26: _t->restartLevel(); break;
        case 27: _t->nextLevel(); break;
        case 28: _t->onBusTapped((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 29: _t->requestUndo(); break;
        case 30: _t->requestHint(); break;
        case 31: _t->onTimerTick(); break;
        case 32: { int _r = _t->boardedCountFor((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 33: { int _r = _t->starsForLevel((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<int*>(_a[0]) = std::move(_r); }  break;
        case 34: { bool _r = _t->canBusMove((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 11:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< MoveOutcome >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::gameStateChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::currentLevelChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::moveCountChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::elapsedSecondsChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::remainingSecondsChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::remainingPassengersChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::remainingBusesChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::currentScoreChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::progressChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::undoAvailabilityChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::hintAvailabilityChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(int , MoveOutcome )>(_a, &GameController::busTapped, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(int , int , int , int , int )>(_a, &GameController::busMoved, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(int , int )>(_a, &GameController::passengerBoarded, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(int )>(_a, &GameController::busLeft, 14))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(LevelResult )>(_a, &GameController::levelWon, 15))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(LevelResult )>(_a, &GameController::levelLost, 16))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(int , int )>(_a, &GameController::timerTick, 17))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::hintRequested, 18))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)()>(_a, &GameController::progressSaved, 19))
            return;
        if (QtMocHelpers::indexOfMethod<void (GameController::*)(QString )>(_a, &GameController::errorOccurred, 20))
            return;
    }
    if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< GameState >(); break;
        case 13:
        case 12:
        case 11:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractListModel* >(); break;
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<GameState*>(_v) = _t->gameState(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->currentLevelId(); break;
        case 2: *reinterpret_cast<int*>(_v) = _t->moveCount(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->elapsedSeconds(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->remainingSeconds(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->remainingPassengers(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->remainingBuses(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->currentScore(); break;
        case 8: *reinterpret_cast<int*>(_v) = _t->highestUnlockedLevel(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->canUndo(); break;
        case 10: *reinterpret_cast<bool*>(_v) = _t->canHint(); break;
        case 11: *reinterpret_cast<QAbstractListModel**>(_v) = _t->busModel(); break;
        case 12: *reinterpret_cast<QAbstractListModel**>(_v) = _t->passengerModel(); break;
        case 13: *reinterpret_cast<QAbstractListModel**>(_v) = _t->platformModel(); break;
        case 14: *reinterpret_cast<int*>(_v) = _t->boardRows(); break;
        case 15: *reinterpret_cast<int*>(_v) = _t->boardCols(); break;
        default: break;
        }
    }
}

const QMetaObject *GameController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GameController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14GameControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GameController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 35)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 35;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 35)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 35;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void GameController::gameStateChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GameController::currentLevelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void GameController::moveCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GameController::elapsedSecondsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void GameController::remainingSecondsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void GameController::remainingPassengersChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void GameController::remainingBusesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void GameController::currentScoreChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void GameController::progressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void GameController::undoAvailabilityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void GameController::hintAvailabilityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void GameController::busTapped(int _t1, MoveOutcome _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1, _t2);
}

// SIGNAL 12
void GameController::busMoved(int _t1, int _t2, int _t3, int _t4, int _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 12, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 13
void GameController::passengerBoarded(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 13, nullptr, _t1, _t2);
}

// SIGNAL 14
void GameController::busLeft(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 14, nullptr, _t1);
}

// SIGNAL 15
void GameController::levelWon(LevelResult _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 15, nullptr, _t1);
}

// SIGNAL 16
void GameController::levelLost(LevelResult _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 16, nullptr, _t1);
}

// SIGNAL 17
void GameController::timerTick(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 17, nullptr, _t1, _t2);
}

// SIGNAL 18
void GameController::hintRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 18, nullptr);
}

// SIGNAL 19
void GameController::progressSaved()
{
    QMetaObject::activate(this, &staticMetaObject, 19, nullptr);
}

// SIGNAL 20
void GameController::errorOccurred(QString _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 20, nullptr, _t1);
}
QT_WARNING_POP
