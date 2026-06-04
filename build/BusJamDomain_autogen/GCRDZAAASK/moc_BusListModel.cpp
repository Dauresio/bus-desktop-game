/****************************************************************************
** Meta object code from reading C++ file 'BusListModel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/engine/BusListModel.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BusListModel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN12BusListModelE_t {};
} // unnamed namespace

template <> constexpr inline auto BusListModel::qt_create_metaobjectdata<qt_meta_tag_ZN12BusListModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BusListModel",
        "BusRole",
        "BusIdRole",
        "ColorIndexRole",
        "CapacityRole",
        "BoardedCountRole",
        "BoardRowRole",
        "BoardColRole",
        "DirectionEnumRole",
        "IsActiveRole",
        "IsAtPlatformRole",
        "HasLeftRole"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'BusRole'
        QtMocHelpers::EnumData<enum BusRole>(1, 1, QMC::EnumFlags{}).add({
            {    2, BusRole::BusIdRole },
            {    3, BusRole::ColorIndexRole },
            {    4, BusRole::CapacityRole },
            {    5, BusRole::BoardedCountRole },
            {    6, BusRole::BoardRowRole },
            {    7, BusRole::BoardColRole },
            {    8, BusRole::DirectionEnumRole },
            {    9, BusRole::IsActiveRole },
            {   10, BusRole::IsAtPlatformRole },
            {   11, BusRole::HasLeftRole },
        }),
    };
    return QtMocHelpers::metaObjectData<BusListModel, qt_meta_tag_ZN12BusListModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BusListModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12BusListModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12BusListModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12BusListModelE_t>.metaTypes,
    nullptr
} };

void BusListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BusListModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *BusListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BusListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12BusListModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int BusListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
