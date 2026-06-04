/****************************************************************************
** Meta object code from reading C++ file 'Enums.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/common/Enums.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Enums.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN11BusJamEnumsE_t {};
} // unnamed namespace

template <> constexpr inline auto BusJamEnums::qt_create_metaobjectdata<qt_meta_tag_ZN11BusJamEnumsE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BusJamEnums",
        "BusColor",
        "Red",
        "Blue",
        "Green",
        "Yellow",
        "Purple",
        "Direction",
        "Up",
        "Down",
        "Left",
        "Right",
        "PassengerState",
        "Waiting",
        "Boarding",
        "Boarded",
        "PlatformState",
        "Free",
        "Occupied",
        "Completed",
        "GameState",
        "Menu",
        "LevelSelection",
        "Playing",
        "Paused",
        "Won",
        "Lost",
        "MoveOutcome",
        "MoveSuccess",
        "MoveBlocked",
        "MoveAlreadyLeft",
        "MoveBusNotFound",
        "MoveNoPlatform",
        "MoveNotPlaying"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'BusColor'
        QtMocHelpers::EnumData<enum BusColor>(1, 1, QMC::EnumFlags{}).add({
            {    2, BusColor::Red },
            {    3, BusColor::Blue },
            {    4, BusColor::Green },
            {    5, BusColor::Yellow },
            {    6, BusColor::Purple },
        }),
        // enum 'Direction'
        QtMocHelpers::EnumData<enum Direction>(7, 7, QMC::EnumFlags{}).add({
            {    8, Direction::Up },
            {    9, Direction::Down },
            {   10, Direction::Left },
            {   11, Direction::Right },
        }),
        // enum 'PassengerState'
        QtMocHelpers::EnumData<enum PassengerState>(12, 12, QMC::EnumFlags{}).add({
            {   13, PassengerState::Waiting },
            {   14, PassengerState::Boarding },
            {   15, PassengerState::Boarded },
        }),
        // enum 'PlatformState'
        QtMocHelpers::EnumData<enum PlatformState>(16, 16, QMC::EnumFlags{}).add({
            {   17, PlatformState::Free },
            {   18, PlatformState::Occupied },
            {   19, PlatformState::Completed },
        }),
        // enum 'GameState'
        QtMocHelpers::EnumData<enum GameState>(20, 20, QMC::EnumFlags{}).add({
            {   21, GameState::Menu },
            {   22, GameState::LevelSelection },
            {   23, GameState::Playing },
            {   24, GameState::Paused },
            {   25, GameState::Won },
            {   26, GameState::Lost },
        }),
        // enum 'MoveOutcome'
        QtMocHelpers::EnumData<enum MoveOutcome>(27, 27, QMC::EnumFlags{}).add({
            {   28, MoveOutcome::MoveSuccess },
            {   29, MoveOutcome::MoveBlocked },
            {   30, MoveOutcome::MoveAlreadyLeft },
            {   31, MoveOutcome::MoveBusNotFound },
            {   32, MoveOutcome::MoveNoPlatform },
            {   33, MoveOutcome::MoveNotPlaying },
        }),
    };
    return QtMocHelpers::metaObjectData<BusJamEnums, qt_meta_tag_ZN11BusJamEnumsE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BusJamEnums::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11BusJamEnumsE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11BusJamEnumsE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN11BusJamEnumsE_t>.metaTypes,
    nullptr
} };

void BusJamEnums::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BusJamEnums *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *BusJamEnums::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BusJamEnums::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN11BusJamEnumsE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BusJamEnums::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
