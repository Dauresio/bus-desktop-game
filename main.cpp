/**
 * @file main.cpp
 * @brief Ponto de entrada da aplicação Bus Jam Puzzle.
 *
 * Responsabilidades:
 *  1. Criar QGuiApplication com metadados da aplicação.
 *  2. Instanciar e ligar as dependências (PersistenceManager,
 *     LevelManager, GameController).
 *  3. Registar tipos C++ no motor QML.
 *  4. Expor objectos via QQmlContext como context properties.
 *  5. Carregar main.qml e entrar no loop de eventos.
 *
 * ### Ordem de inicialização:
 *  PersistenceManager → LevelManager → GameController → QQmlEngine
 *
 *  A ordem importa: GameController recebe ponteiros para os dois primeiros;
 *  o engine QML é o último a ser criado para que todas as context properties
 *  estejam disponíveis quando o QML é avaliado.
 *
 * ### Fontes cartoon:
 *  Fredoka One e Nunito são carregadas dos recursos Qt (:/fonts/).
 *  Se não estiverem disponíveis, o sistema usa a fonte sans-serif padrão
 *  — o jogo corre mas sem a tipografia cartoon pretendida.
 */

#include "src/engine/GameController.h"
#include "src/persistence/LevelManager.h"
#include "src/persistence/PersistenceManager.h"
#include "src/common/Enums.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QFontDatabase>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QIcon>

// ─────────────────────────────────────────────────────────────────────────────
// Constantes da aplicação
// ─────────────────────────────────────────────────────────────────────────────

namespace AppInfo
{
    constexpr const char* kOrganization    = "BusJamStudio";
    constexpr const char* kApplication    = "BusJamPuzzle";
    constexpr const char* kVersion        = "1.0.0";
    constexpr const char* kQmlMain        = "qrc:/BusJamPuzzle/qml/main.qml";
    constexpr const char* kLevelsDir      = ":/levels";   // Recursos Qt
    constexpr const char* kQmlUri         = "BusJam";
    constexpr int         kQmlMajor       = 1;
    constexpr int         kQmlMinor       = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Funções auxiliares
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Carrega fontes TTF/OTF dos recursos Qt para o QFontDatabase.
 *
 * Procura em ":/fonts/" por todos os ficheiros *.ttf e *.otf e regista-os.
 * Se a directoria não existir ou estiver vazia, regista apenas aviso.
 */
static void loadFonts()
{
    const QStringList fontFiles = {
        QStringLiteral(":/fonts/FredokaOne-Regular.ttf"),
        QStringLiteral(":/fonts/Nunito-Regular.ttf"),
        QStringLiteral(":/fonts/Nunito-Bold.ttf")
    };

    int loaded = 0;
    for (const QString& path : fontFiles)
    {
        const int id = QFontDatabase::addApplicationFont(path);
        if (id >= 0)
        {
            ++loaded;
            qDebug().noquote() << "Fonte carregada:" << path
                               << "→" << QFontDatabase::applicationFontFamilies(id);
        }
        else
        {
            qWarning().noquote()
                << "Não foi possível carregar fonte:" << path
                << "(o jogo usa a fonte de sistema como fallback)";
        }
    }

    qDebug().noquote() << loaded << "fonte(s) carregada(s).";
}

/**
 * @brief Regista os tipos C++ no motor QML.
 *
 * - BusJamEnums → namespace de enums acessível como "BusJam.Red", etc.
 * - GameController, LevelManager, PersistenceManager → expostos via
 *   context properties (não como tipos QML instanciáveis).
 */
static void registerQmlTypes()
{
    // Enums partilhados — acessíveis em QML como BusJam.Red, BusJam.Playing, etc.
    qmlRegisterUncreatableType<BusJamEnums>(
        AppInfo::kQmlUri,
        AppInfo::kQmlMajor,
        AppInfo::kQmlMinor,
        "BusJam",
        QStringLiteral("BusJam é apenas um namespace de enums — não instanciável."));

    // LevelResult precisa de ser conhecido pelo sistema de metatypes para
    // poder ser passado em sinais Qt::QueuedConnection e para QML via variante.
    qRegisterMetaType<LevelResult>("LevelResult");

    qDebug() << "Tipos QML registados.";
}

/**
 * @brief Inicializa o LevelManager a partir dos recursos Qt ou de um
 *        directório externo (para desenvolvimento).
 *
 * Tenta primeiro recursos Qt (produção). Se vazios, tenta directório
 * relativo ao executável (desenvolvimento com ficheiros externos).
 *
 * @param manager LevelManager a inicializar.
 * @return true se pelo menos um nível foi carregado.
 */
static bool initLevelManager(LevelManager& manager)
{
    // Tentativa 1: recursos Qt (compilados no binário).
    ManagerResult result = manager.loadFromResources();

    if (result.success)
    {
        qDebug().noquote()
            << "LevelManager: níveis carregados dos recursos Qt."
            << manager.toString();
        return true;
    }

    qDebug().noquote()
        << "LevelManager: recursos Qt vazios:" << result.errorMessage
        << "— a tentar directório externo.";

    // Tentativa 2: directório "levels/" junto ao executável.
    const QString exeDir = QCoreApplication::applicationDirPath();
    const QString levelsDir = exeDir + QStringLiteral("/levels");

    result = manager.loadFromDirectory(levelsDir);

    if (result.success)
    {
        qDebug().noquote()
            << "LevelManager: níveis carregados de" << levelsDir
            << manager.toString();
        return true;
    }

    // Tentativa 3: directório de trabalho actual (útil em testes).
    const QString cwdLevels = QDir::currentPath() + QStringLiteral("/levels");
    result = manager.loadFromDirectory(cwdLevels);

    if (result.success)
    {
        qDebug().noquote()
            << "LevelManager: níveis carregados de" << cwdLevels;
        return true;
    }

    qCritical().noquote()
        << "LevelManager: NENHUM nível encontrado."
        << "Coloque ficheiros JSON em levels/ junto ao executável,"
        << "ou inclua-os como recursos Qt em :/levels/.";

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    // ── Aplicação ─────────────────────────────────────────────────────────
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QString::fromLatin1(AppInfo::kOrganization));
    app.setOrganizationDomain(QStringLiteral("busjampuzzle.example.com"));
    app.setApplicationName(QString::fromLatin1(AppInfo::kApplication));
    app.setApplicationVersion(QString::fromLatin1(AppInfo::kVersion));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app_icon.png")));

    qDebug().noquote()
        << app.applicationName() << app.applicationVersion()
        << "— a iniciar.";

    // ── Fontes ────────────────────────────────────────────────────────────
    loadFonts();

    // ── Registo de tipos QML ──────────────────────────────────────────────
    registerQmlTypes();

    // ── Camada de persistência ────────────────────────────────────────────
    PersistenceManager persistenceManager(
        QString::fromLatin1(AppInfo::kOrganization),
        QString::fromLatin1(AppInfo::kApplication));

    // ── Gestor de níveis ──────────────────────────────────────────────────
    LevelManager levelManager;

    const bool levelsOk = initLevelManager(levelManager);
    if (!levelsOk)
    {
        // Em modo de desenvolvimento, continuar sem níveis para permitir
        // que a UI arranque. Em produção, seria um erro fatal.
        qWarning() << "A continuar sem níveis (modo de desenvolvimento).";
    }

    // ── Controlador de jogo ───────────────────────────────────────────────
    GameController gameController(&levelManager, &persistenceManager);

    // ── Motor QML ─────────────────────────────────────────────────────────
    QQmlApplicationEngine engine;

    // Context properties — acessíveis em todo o QML como variáveis globais.
    QQmlContext* rootContext = engine.rootContext();
    rootContext->setContextProperty(QStringLiteral("gameController"),  &gameController);

    // Caminho de recursos base para o QML importar ficheiros locais.
    engine.addImportPath(QStringLiteral("qrc:/"));

    // ── Carregar QML principal ────────────────────────────────────────────
    const QUrl mainQml(QString::fromLatin1(AppInfo::kQmlMain));

    // Ligar erro de carregamento do QML a saída imediata (falha rápida).
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [mainQml](QObject* obj, const QUrl& objUrl) {
            if (!obj && objUrl == mainQml)
            {
                qCritical().noquote()
                    << "Falha ao carregar" << mainQml.toString()
                    << "— a terminar.";
                QCoreApplication::exit(EXIT_FAILURE);
            }
        },
        Qt::QueuedConnection);

    engine.load(mainQml);

    if (engine.rootObjects().isEmpty())
    {
        qCritical() << "Engine QML não tem objectos raiz — a terminar.";
        return EXIT_FAILURE;
    }

    qDebug().noquote()
        << app.applicationName() << "iniciado com sucesso."
        << "Níveis disponíveis:" << levelManager.levelCount();

    // ── Loop de eventos ───────────────────────────────────────────────────
    const int exitCode = app.exec();

    qDebug().noquote()
        << app.applicationName() << "terminado com código" << exitCode;

    return exitCode;
}
