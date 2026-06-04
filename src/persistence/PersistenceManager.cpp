/**
 * @file PersistenceManager.cpp
 * @brief Implementação da classe PersistenceManager para o jogo Bus Jam Puzzle.
 *
 * Dois backends de armazenamento cooperam:
 *
 *  QSettings    → chaves simples (progresso, definições, sessão, estatísticas).
 *                 Escrita lazy; flush() força escrita imediata.
 *
 *  JSON em disco → QHash<int, SavedLevelResult> serializado.
 *                 Lido uma vez (lazy) para resultsCache_; escrito por
 *                 saveResultsToDisk() após cada update.
 *
 * A cache resultsCache_ é marcada como mutable porque loadResultsFromDisk()
 * é chamado de métodos const (ex.: loadLevelResult()). O invariante é:
 * após qualquer chamada a um getter de resultados, resultsCache_ está
 * sincronizado com o disco.
 */

#include "PersistenceManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <algorithm>   // std::sort

// ─────────────────────────────────────────────────────────────────────────────
// Construtor e destrutor
// ─────────────────────────────────────────────────────────────────────────────

PersistenceManager::PersistenceManager(const QString& organizationName,
                                       const QString& applicationName)
    : settings_       { QSettings::UserScope, organizationName, applicationName }
    , resultsFilePath_{ resolveResultsFilePath(applicationName) }
    , cacheLoaded_    { false }
{
    migrateIfNeeded();
    initializeDefaults();

    qDebug().noquote()
        << "PersistenceManager criado."
        << "\n  QSettings :" << settings_.fileName()
        << "\n  Results   :" << resultsFilePath_
        << "\n  Estado    :" << toString();
}

PersistenceManager::~PersistenceManager()
{
    flush();
    qDebug() << "PersistenceManager destruído. flush() executado.";
}

// ─────────────────────────────────────────────────────────────────────────────
// Progresso
// ─────────────────────────────────────────────────────────────────────────────

int PersistenceManager::loadHighestUnlockedLevel() const
{
    return settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyHighestLevel), 1).toInt();
}

bool PersistenceManager::saveHighestUnlockedLevel(int newLevel)
{
    if (newLevel < 1)
    {
        qWarning().noquote()
            << "PersistenceManager::saveHighestUnlockedLevel: id inválido"
            << newLevel;
        return false;
    }

    const int current = loadHighestUnlockedLevel();

    if (newLevel <= current)
        return false;  // Nunca regride.

    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyHighestLevel), newLevel);

    qDebug().noquote()
        << "PersistenceManager: highestUnlockedLevel" << current
        << "→" << newLevel;

    return true;
}

int PersistenceManager::loadLastPlayedLevel() const
{
    return settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyLastPlayedLevel), -1).toInt();
}

void PersistenceManager::saveLastPlayedLevel(int levelId)
{
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyLastPlayedLevel), levelId);
}

// ─────────────────────────────────────────────────────────────────────────────
// Resultados por nível
// ─────────────────────────────────────────────────────────────────────────────

SavedLevelResult PersistenceManager::loadLevelResult(int levelId) const
{
    loadResultsFromDisk();  // No-op se já carregado.

    if (!resultsCache_.contains(levelId))
    {
        SavedLevelResult empty;
        empty.levelId = levelId;
        return empty;
    }

    return resultsCache_.value(levelId);
}

bool PersistenceManager::saveLevelResult(int levelId,
                                          int score,
                                          int moves,
                                          int secs,
                                          int stars)
{
    if (levelId < 1)
    {
        qWarning().noquote()
            << "PersistenceManager::saveLevelResult: levelId inválido" << levelId;
        return false;
    }

    loadResultsFromDisk();

    SavedLevelResult& r = resultsCache_[levelId];
    r.levelId = levelId;

    bool improved = false;

    // Score — maior é melhor.
    if (score > r.bestScore)
    {
        r.bestScore = score;
        improved = true;
    }

    // Moves — menor é melhor (INT_MAX = nunca completado).
    if (moves < r.bestMoves)
    {
        r.bestMoves = moves;
        improved = true;
    }

    // Tempo — menor é melhor.
    if (secs < r.bestTimeSecs)
    {
        r.bestTimeSecs = secs;
        improved = true;
    }

    // Estrelas — maior é melhor.
    if (stars > r.stars)
    {
        r.stars = stars;
        improved = true;
    }

    // Sempre acumula.
    ++r.completionCount;
    r.lastPlayed = QDateTime::currentDateTime();

    // Estatísticas globais.
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyTotalCompletions),
        loadTotalCompletions() + 1);

    const bool written = saveResultsToDisk();

    if (!written)
    {
        qWarning().noquote()
            << "PersistenceManager::saveLevelResult: falha ao escrever para disco."
            << "Resultado guardado apenas em cache.";
    }

    qDebug().noquote()
        << "PersistenceManager: nível" << levelId
        << "resultado guardado."
        << "score=" << score
        << "moves=" << moves
        << "stars=" << stars
        << "improved=" << improved;

    return improved;
}

bool PersistenceManager::isLevelCompleted(int levelId) const
{
    const SavedLevelResult r = loadLevelResult(levelId);
    return r.completionCount > 0;
}

int PersistenceManager::starsForLevel(int levelId) const
{
    return loadLevelResult(levelId).stars;
}

QList<SavedLevelResult> PersistenceManager::loadAllLevelResults() const
{
    loadResultsFromDisk();

    QList<SavedLevelResult> results;
    results.reserve(resultsCache_.size());

    for (const auto& r : resultsCache_)
        results.append(r);

    // Ordena por levelId crescente.
    std::sort(results.begin(), results.end(),
              [](const SavedLevelResult& a, const SavedLevelResult& b) {
                  return a.levelId < b.levelId;
              });

    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// Definições
// ─────────────────────────────────────────────────────────────────────────────

GameSettings PersistenceManager::loadSettings() const
{
    GameSettings s;

    s.soundEnabled = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeySoundEnabled), true).toBool();

    s.musicEnabled = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyMusicEnabled), true).toBool();

    s.language = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyLanguage),
        QStringLiteral("pt")).toString();

    s.colorTheme = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyColorTheme),
        QStringLiteral("default")).toString();

    s.masterVolume = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyMasterVolume), 100).toInt();

    return s;
}

void PersistenceManager::saveSettings(const GameSettings& s)
{
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeySoundEnabled),   s.soundEnabled);
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyMusicEnabled),   s.musicEnabled);
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyLanguage),       s.language);
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyColorTheme),     s.colorTheme);
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyMasterVolume),
        qBound(0, s.masterVolume, 100));

    qDebug().noquote()
        << "PersistenceManager: definições guardadas."
        << "sound=" << s.soundEnabled
        << "music=" << s.musicEnabled
        << "lang=" << s.language;
}

void PersistenceManager::saveSoundEnabled(bool enabled)
{
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeySoundEnabled), enabled);
}

void PersistenceManager::saveMusicEnabled(bool enabled)
{
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyMusicEnabled), enabled);
}

// ─────────────────────────────────────────────────────────────────────────────
// Estado de sessão
// ─────────────────────────────────────────────────────────────────────────────

void PersistenceManager::saveSessionState(const SessionState& state)
{
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeySessionLevelId),
        state.levelId);
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeySessionMoveCount),
        state.moveCount);
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeySessionElapsedSecs),
        state.elapsedSecs);
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeySessionIsActive),
        state.isActive);

    // flush() imediato: estado de sessão deve sobreviver a um crash.
    settings_.sync();

    qDebug().noquote()
        << "PersistenceManager: sessão guardada."
        << "level=" << state.levelId
        << "moves=" << state.moveCount
        << "elapsed=" << state.elapsedSecs << "s";
}

SessionState PersistenceManager::loadSessionState() const
{
    SessionState state;

    state.isActive = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeySessionIsActive), false).toBool();

    if (!state.isActive)
        return state;  // Sem sessão activa — devolve estado vazio.

    state.levelId = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeySessionLevelId), -1).toInt();
    state.moveCount = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeySessionMoveCount), 0).toInt();
    state.elapsedSecs = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeySessionElapsedSecs), 0).toInt();

    return state;
}

void PersistenceManager::clearSessionState()
{
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeySessionIsActive), false);
    settings_.remove(
        QString::fromLatin1(PersistenceConstants::kKeySessionLevelId));
    settings_.remove(
        QString::fromLatin1(PersistenceConstants::kKeySessionMoveCount));
    settings_.remove(
        QString::fromLatin1(PersistenceConstants::kKeySessionElapsedSecs));

    qDebug() << "PersistenceManager: estado de sessão limpo.";
}

// ─────────────────────────────────────────────────────────────────────────────
// Estatísticas globais
// ─────────────────────────────────────────────────────────────────────────────

qint64 PersistenceManager::loadTotalPlaytimeSecs() const
{
    return settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyTotalPlaytimeSecs),
        0LL).toLongLong();
}

void PersistenceManager::addPlaytimeSecs(qint64 secs)
{
    if (secs <= 0)
        return;

    const qint64 total = loadTotalPlaytimeSecs() + secs;
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyTotalPlaytimeSecs), total);
}

int PersistenceManager::loadTotalCompletions() const
{
    return settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyTotalCompletions), 0).toInt();
}

// ─────────────────────────────────────────────────────────────────────────────
// Reset
// ─────────────────────────────────────────────────────────────────────────────

void PersistenceManager::resetProgress()
{
    // Preservar definições antes de limpar.
    const GameSettings savedSettings = loadSettings();

    // Remover apenas chaves de progresso e resultados.
    settings_.remove(QStringLiteral("progress"));
    settings_.remove(QStringLiteral("session"));
    settings_.remove(QStringLiteral("stats"));

    // Apagar ficheiro de resultados JSON.
    QFile resultsFile(resultsFilePath_);
    if (resultsFile.exists())
        resultsFile.remove();

    // Limpar cache.
    resultsCache_.clear();
    cacheLoaded_ = false;

    // Restaurar definições.
    saveSettings(savedSettings);

    // Repor valores por omissão de progresso.
    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyHighestLevel), 1);

    flush();

    qDebug() << "PersistenceManager::resetProgress: progresso apagado."
             << "Definições preservadas.";
}

void PersistenceManager::resetAll()
{
    settings_.clear();

    QFile resultsFile(resultsFilePath_);
    if (resultsFile.exists())
        resultsFile.remove();

    resultsCache_.clear();
    cacheLoaded_ = false;

    initializeDefaults();
    flush();

    qDebug() << "PersistenceManager::resetAll: todos os dados apagados.";
}

// ─────────────────────────────────────────────────────────────────────────────
// I/O e sincronização
// ─────────────────────────────────────────────────────────────────────────────

void PersistenceManager::flush()
{
    settings_.sync();
    saveResultsToDisk();
}

QString PersistenceManager::settingsFilePath() const
{
    return settings_.fileName();
}

QString PersistenceManager::resultsFilePath() const
{
    return resultsFilePath_;
}

bool PersistenceManager::isDataIntact() const
{
    const int version = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyDataVersion), -1).toInt();

    if (version != PersistenceConstants::kCurrentDataVersion)
        return false;

    // Verificar integridade do JSON de resultados (se existir).
    QFile f(resultsFilePath_);
    if (!f.exists())
        return true;  // Sem ficheiro é válido (nenhum nível completado ainda).

    if (!f.open(QIODevice::ReadOnly))
        return false;

    const QByteArray data = f.readAll();
    f.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    return err.error == QJsonParseError::NoError && doc.isObject();
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString PersistenceManager::toString() const
{
    const SessionState session = loadSessionState();
    return QString("PersistenceManager[highestLevel=%1 results=%2 sessionActive=%3]")
        .arg(loadHighestUnlockedLevel())
        .arg(resultsCache_.size())
        .arg(session.isActive ? "true" : "false");
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

QString PersistenceManager::resolveResultsFilePath(const QString& appName)
{
    const QString dataDir = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);

    QDir dir(dataDir);
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    return dir.filePath(appName + QStringLiteral("_results.json"));
}

void PersistenceManager::loadResultsFromDisk() const
{
    if (cacheLoaded_)
        return;

    cacheLoaded_ = true;  // Marcado antes da leitura para evitar recursão.
    resultsCache_.clear();

    QFile f(resultsFilePath_);
    if (!f.exists())
        return;  // Ficheiro ainda não criado — cache vazia é válida.

    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning().noquote()
            << "PersistenceManager: não foi possível ler" << resultsFilePath_;
        return;
    }

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();

    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        qWarning().noquote()
            << "PersistenceManager: ficheiro de resultados corrompido:"
            << resultsFilePath_ << "—" << err.errorString();
        return;
    }

    const QJsonObject root = doc.object();

    // Iterar sobre todas as chaves (cada chave é um levelId em string).
    for (auto it = root.constBegin(); it != root.constEnd(); ++it)
    {
        bool ok = false;
        const int levelId = it.key().toInt(&ok);

        if (!ok || !it.value().isObject())
            continue;

        const SavedLevelResult r = resultFromJson(it.value().toObject(), levelId);
        if (r.isValid())
            resultsCache_.insert(levelId, r);
    }

    qDebug().noquote()
        << "PersistenceManager: carregados"
        << resultsCache_.size() << "resultados de" << resultsFilePath_;
}

bool PersistenceManager::saveResultsToDisk() const
{
    QJsonObject root;

    for (auto it = resultsCache_.constBegin(); it != resultsCache_.constEnd(); ++it)
    {
        root[QString::number(it.key())] = resultToJson(it.value());
    }

    const QJsonDocument doc(root);
    QFile f(resultsFilePath_);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qWarning().noquote()
            << "PersistenceManager: não foi possível escrever" << resultsFilePath_;
        return false;
    }

    const qint64 written = f.write(doc.toJson(QJsonDocument::Indented));
    f.close();

    if (written < 0)
    {
        qWarning().noquote()
            << "PersistenceManager: erro de escrita em" << resultsFilePath_;
        return false;
    }

    return true;
}

QJsonObject PersistenceManager::resultToJson(const SavedLevelResult& r)
{
    QJsonObject obj;
    obj[QStringLiteral("levelId")]         = r.levelId;
    obj[QStringLiteral("bestScore")]       = r.bestScore;
    obj[QStringLiteral("bestMoves")]       = r.bestMoves == INT_MAX ? -1 : r.bestMoves;
    obj[QStringLiteral("bestTimeSecs")]    = r.bestTimeSecs == INT_MAX ? -1 : r.bestTimeSecs;
    obj[QStringLiteral("stars")]           = r.stars;
    obj[QStringLiteral("completionCount")] = r.completionCount;
    obj[QStringLiteral("lastPlayed")]      = r.lastPlayed.toString(Qt::ISODate);
    return obj;
}

SavedLevelResult PersistenceManager::resultFromJson(const QJsonObject& obj, int levelId)
{
    SavedLevelResult r;
    r.levelId = levelId;

    r.bestScore       = obj.value(QStringLiteral("bestScore")).toInt(0);
    r.stars           = obj.value(QStringLiteral("stars")).toInt(0);
    r.completionCount = obj.value(QStringLiteral("completionCount")).toInt(0);

    const int moves = obj.value(QStringLiteral("bestMoves")).toInt(-1);
    r.bestMoves = (moves < 0) ? INT_MAX : moves;

    const int timeSecs = obj.value(QStringLiteral("bestTimeSecs")).toInt(-1);
    r.bestTimeSecs = (timeSecs < 0) ? INT_MAX : timeSecs;

    const QString dateStr = obj.value(QStringLiteral("lastPlayed")).toString();
    if (!dateStr.isEmpty())
        r.lastPlayed = QDateTime::fromString(dateStr, Qt::ISODate);

    return r;
}

void PersistenceManager::migrateIfNeeded()
{
    const int storedVersion = settings_.value(
        QString::fromLatin1(PersistenceConstants::kKeyDataVersion), 0).toInt();

    if (storedVersion == PersistenceConstants::kCurrentDataVersion)
        return;

    if (storedVersion == 0)
    {
        // Primeira instalação — sem migração necessária, só inicializar.
        qDebug() << "PersistenceManager: primeira inicialização.";
    }
    else
    {
        // Futuros casos de migração virão aqui:
        // if (storedVersion < 2) { migrateTo2(); }
        // if (storedVersion < 3) { migrateTo3(); }
        qWarning().noquote()
            << "PersistenceManager: versão de dados desconhecida ("
            << storedVersion << "). A repor para" << PersistenceConstants::kCurrentDataVersion;
    }

    settings_.setValue(
        QString::fromLatin1(PersistenceConstants::kKeyDataVersion),
        PersistenceConstants::kCurrentDataVersion);
}

void PersistenceManager::initializeDefaults()
{
    // Só escreve se a chave ainda não existir (não sobrescreve dados guardados).
    auto setDefault = [this](const char* key, const QVariant& value) {
        if (!settings_.contains(QString::fromLatin1(key)))
            settings_.setValue(QString::fromLatin1(key), value);
    };

    setDefault(PersistenceConstants::kKeyHighestLevel,    1);
    setDefault(PersistenceConstants::kKeyLastPlayedLevel, -1);
    setDefault(PersistenceConstants::kKeySoundEnabled,    true);
    setDefault(PersistenceConstants::kKeyMusicEnabled,    true);
    setDefault(PersistenceConstants::kKeyLanguage,        QStringLiteral("pt"));
    setDefault(PersistenceConstants::kKeyColorTheme,      QStringLiteral("default"));
    setDefault(PersistenceConstants::kKeyMasterVolume,    100);
    setDefault(PersistenceConstants::kKeySessionIsActive, false);
    setDefault(PersistenceConstants::kKeyTotalPlaytimeSecs, 0LL);
    setDefault(PersistenceConstants::kKeyTotalCompletions,  0);
}
