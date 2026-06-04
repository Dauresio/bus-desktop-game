/****************************************************************************
** Meta object code from reading C++ file 'PlatformListModel.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/engine/PlatformListModel.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PlatformListModel.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN17PlatformListModelE_t {};
} // unnamed namespace

template <> constexpr inline auto PlatformListModel::qt_create_metaobjectdata<qt_meta_tag_ZN17PlatformListModelE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "PlatformListModel",
        "PlatformRole",
        "PlatformIdRole",
        "ColorIndexRole",
        "PlatformRowRole",
        "PlatformColRole",
        "ExitDirEnumRole",
        "StateEnumRole",
        "IsFreeRole",
        "IsOccupiedRole",
        "IsCompletedRole"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'PlatformRole'
        QtMocHelpers::EnumData<enum PlatformRole>(1, 1, QMC::EnumFlags{}).add({
            {    2, PlatformRole::PlatformIdRole },
            {    3, PlatformRole::ColorIndexRole },
            {    4, PlatformRole::PlatformRowRole },
            {    5, PlatformRole::PlatformColRole },
            {    6, PlatformRole::ExitDirEnumRole },
            {    7, PlatformRole::StateEnumRole },
            {    8, PlatformRole::IsFreeRole },
            {    9, PlatformRole::IsOccupiedRole },
            {   10, PlatformRole::IsCompletedRole },
        }),
    };
    return QtMocHelpers::metaObjectData<PlatformListModel, qt_meta_tag_ZN17PlatformListModelE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject PlatformListModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17PlatformListModelE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17PlatformListModelE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN17PlatformListModelE_t>.metaTypes,
    nullptr
} };

void PlatformListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<PlatformListModel *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *PlatformListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PlatformListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN17PlatformListModelE_t>.strings))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int PlatformListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
