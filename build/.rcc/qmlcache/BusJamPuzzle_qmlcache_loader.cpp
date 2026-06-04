#include <QtQml/qqmlprivate.h>
#include <QtCore/qdir.h>
#include <QtCore/qurl.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

namespace QmlCacheGeneratedCode {
namespace _0x5f_BusJamPuzzle_qml_main_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_screens_MainMenuScreen_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_screens_LevelSelectScreen_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_screens_GameScreen_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_components_BoardView_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_components_BusItem_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_components_PassengerItem_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_components_PlatformView_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_components_HUD_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_style_Theme_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _0x5f_BusJamPuzzle_qml_style_Sizes_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}

}
namespace {
struct Registry {
    Registry();
    ~Registry();
    QHash<QString, const QQmlPrivate::CachedQmlUnit*> resourcePathToCachedUnit;
    static const QQmlPrivate::CachedQmlUnit *lookupCachedUnit(const QUrl &url);
};

Q_GLOBAL_STATIC(Registry, unitRegistry)


Registry::Registry() {
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/main.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_main_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/screens/MainMenuScreen.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_screens_MainMenuScreen_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/screens/LevelSelectScreen.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_screens_LevelSelectScreen_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/screens/GameScreen.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_screens_GameScreen_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/components/BoardView.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_components_BoardView_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/components/BusItem.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_components_BusItem_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/components/PassengerItem.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_components_PassengerItem_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/components/PlatformView.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_components_PlatformView_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/components/HUD.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_components_HUD_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/style/Theme.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_style_Theme_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/BusJamPuzzle/qml/style/Sizes.qml"), &QmlCacheGeneratedCode::_0x5f_BusJamPuzzle_qml_style_Sizes_qml::unit);
    QQmlPrivate::RegisterQmlUnitCacheHook registration;
    registration.structVersion = 0;
    registration.lookupCachedQmlUnit = &lookupCachedUnit;
    QQmlPrivate::qmlregister(QQmlPrivate::QmlUnitCacheHookRegistration, &registration);
}

Registry::~Registry() {
    QQmlPrivate::qmlunregister(QQmlPrivate::QmlUnitCacheHookRegistration, quintptr(&lookupCachedUnit));
}

const QQmlPrivate::CachedQmlUnit *Registry::lookupCachedUnit(const QUrl &url) {
    if (url.scheme() != QLatin1String("qrc"))
        return nullptr;
    QString resourcePath = QDir::cleanPath(url.path());
    if (resourcePath.isEmpty())
        return nullptr;
    if (!resourcePath.startsWith(QLatin1Char('/')))
        resourcePath.prepend(QLatin1Char('/'));
    return unitRegistry()->resourcePathToCachedUnit.value(resourcePath, nullptr);
}
}
int QT_MANGLE_NAMESPACE(qInitResources_qmlcache_BusJamPuzzle)() {
    ::unitRegistry();
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qInitResources_qmlcache_BusJamPuzzle))
int QT_MANGLE_NAMESPACE(qCleanupResources_qmlcache_BusJamPuzzle)() {
    return 1;
}
