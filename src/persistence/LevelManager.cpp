/**
 * @file LevelManager.cpp
 * @brief Implementação da classe LevelManager para o jogo Bus Jam Puzzle.
 *
 * O LevelManager opera em dois passos:
 *
 *  1. Fase de descoberta (loadFromDirectory / loadFromResources / addLevel):
 *     Lê apenas os metadados de cada ficheiro. Operação leve — não constrói
 *     nenhum Board. Popula metadataMap_ e sortedIds_.
 *
 *  2. Fase de uso (buildBoardForLevel):
 *     Chamada pelo GameController quando o jogador selecciona um nível.
 *     Constrói o Board completo via LevelLoader::fromFile(). Sempre constrói
 *     de raiz para garantir estado inicial limpo (sem estado de jogada anterior).
 *
 * Ordenação: sortedIds_ é mantida ordenada por id crescente via
 * std::lower_bound após cada inserção — O(log n) por inserção.
 */

#include "LevelManager.h"

#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <algorithm>   // std::lower_bound, std::sort

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────

LevelManager::LevelManager()
{
    qDebug() << "LevelManager criado.";
}

// ─────────────────────────────────────────────────────────────────────────────
// Carregamento de metadados
// ─────────────────────────────────────────────────────────────────────────────

ManagerResult LevelManager::loadFromDirectory(const QString& dirPath)
{
    QDir dir(dirPath);

    if (!dir.exists())
    {
        return ManagerResult::fail(
            QString("LevelManager: diretório '%1' não existe.").arg(dirPath));
    }

    // Percorrer todos os ficheiros *.json no diretório (não recursivo).
    QDirIterator it(dirPath,
                    QStringList{ QStringLiteral("*.json") },
                    QDir::Files,
                    QDirIterator::NoIteratorFlags);

    int loaded  = 0;
    int skipped = 0;

    while (it.hasNext())
    {
        const QString filePath = it.next();
        LevelMetadata meta;

        const LoadResult result = LevelLoader::metadataFromFile(filePath, meta);

        if (!result.success)
        {
            qWarning().noquote()
                << "LevelManager: ignorando ficheiro inválido:"
                << filePath << "—" << result.errorMessage;
            ++skipped;
            continue;
        }

        // Verificar id duplicado.
        if (metadataMap_.contains(meta.id))
        {
            qWarning().noquote()
                << "LevelManager: id" << meta.id << "duplicado em"
                << filePath
                << "(já registado em" << metadataMap_.value(meta.id).filePath << ")."
                << "Ficheiro ignorado.";
            ++skipped;
            continue;
        }

        insertMetadata(meta);
        ++loaded;
    }

    if (loaded == 0)
    {
        return ManagerResult::fail(
            QString("LevelManager: nenhum nível válido encontrado em '%1'."
                    " (%2 ficheiro(s) ignorado(s)).")
                .arg(dirPath)
                .arg(skipped));
    }

    qDebug().noquote()
        << "LevelManager: carregados" << loaded << "nível(eis) de"
        << dirPath << "(" << skipped << "ignorado(s)). " << toString();

    return ManagerResult::ok();
}

ManagerResult LevelManager::loadFromResources()
{
    // Iterar sobre os recursos Qt no prefixo :/levels/
    QDirIterator it(QString::fromLatin1(LevelManagerConstants::kQrcLevelsPrefix),
                    QStringList{ QStringLiteral("*.json") },
                    QDir::Files,
                    QDirIterator::Subdirectories);

    int loaded  = 0;
    int skipped = 0;

    while (it.hasNext())
    {
        const QString resourcePath = it.next();
        LevelMetadata meta;

        const LoadResult result = LevelLoader::metadataFromFile(resourcePath, meta);

        if (!result.success)
        {
            qWarning().noquote()
                << "LevelManager: recurso inválido:" << resourcePath
                << "—" << result.errorMessage;
            ++skipped;
            continue;
        }

        if (metadataMap_.contains(meta.id))
        {
            qWarning().noquote()
                << "LevelManager: id" << meta.id << "duplicado em recurso"
                << resourcePath << ". Ignorado.";
            ++skipped;
            continue;
        }

        insertMetadata(meta);
        ++loaded;
    }

    if (loaded == 0)
    {
        return ManagerResult::fail(
            QStringLiteral("LevelManager: nenhum nível válido nos recursos Qt "
                           "(:/levels/). Verifique o ficheiro .qrc."));
    }

    qDebug().noquote()
        << "LevelManager: carregados" << loaded << "nível(eis) dos recursos Qt."
        << toString();

    return ManagerResult::ok();
}

LoadResult LevelManager::addLevel(const QString& filePath)
{
    LevelMetadata meta;
    LoadResult result = LevelLoader::metadataFromFile(filePath, meta);

    if (!result.success)
        return result;

    if (metadataMap_.contains(meta.id))
    {
        return LoadResult::fail(LoadErrorCode::DuplicateId,
            QString("LevelManager::addLevel: id %1 já existe no catálogo.")
                .arg(meta.id));
    }

    insertMetadata(meta);

    qDebug().noquote()
        << "LevelManager: nível" << meta.id
        << QString("'%1'").arg(meta.name)
        << "adicionado de" << filePath;

    return LoadResult::ok(meta.id);
}

// ─────────────────────────────────────────────────────────────────────────────
// Consultas de catálogo
// ─────────────────────────────────────────────────────────────────────────────

int  LevelManager::levelCount() const noexcept { return metadataMap_.size(); }
bool LevelManager::isEmpty()    const noexcept { return metadataMap_.isEmpty(); }

bool LevelManager::hasLevel(int levelId) const noexcept
{
    return metadataMap_.contains(levelId);
}

const LevelMetadata* LevelManager::metadataForLevel(int levelId) const
{
    auto it = metadataMap_.find(levelId);
    if (it == metadataMap_.end())
        return nullptr;

    return &it.value();
}

QList<LevelMetadata> LevelManager::allLevelMetadata() const
{
    QList<LevelMetadata> result;
    result.reserve(sortedIds_.size());

    for (int id : sortedIds_)
        result.append(metadataMap_.value(id));

    return result;
}

QList<int> LevelManager::allLevelIds() const
{
    return sortedIds_;  // Já ordenada.
}

int LevelManager::firstLevelId() const noexcept
{
    return sortedIds_.isEmpty() ? -1 : sortedIds_.first();
}

int LevelManager::nextLevelId(int currentLevelId) const noexcept
{
    // Pesquisa binária: encontrar o índice do id atual em sortedIds_.
    const auto it = std::lower_bound(sortedIds_.cbegin(),
                                     sortedIds_.cend(),
                                     currentLevelId);

    if (it == sortedIds_.cend())
        return LevelManagerConstants::kNoNextLevel;  // Id não encontrado.

    // Avança um passo.
    const auto next = std::next(it);

    if (next == sortedIds_.cend())
        return LevelManagerConstants::kNoNextLevel;  // Era o último nível.

    return *next;
}

bool LevelManager::hasNextLevel(int currentLevelId) const noexcept
{
    return nextLevelId(currentLevelId) != LevelManagerConstants::kNoNextLevel;
}

int LevelManager::timeLimitForLevel(int levelId) const noexcept
{
    const LevelMetadata* meta = metadataForLevel(levelId);
    return meta ? meta->timeLimitSecs : 0;
}

int LevelManager::difficultyForLevel(int levelId) const noexcept
{
    const LevelMetadata* meta = metadataForLevel(levelId);
    return meta ? meta->difficulty : -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Construção de Board
// ─────────────────────────────────────────────────────────────────────────────

std::unique_ptr<Board> LevelManager::buildBoardForLevel(int         levelId,
                                                         LoadResult& outResult)
{
    // Verificar se o nível existe no catálogo.
    const LevelMetadata* meta = metadataForLevel(levelId);

    if (!meta)
    {
        outResult = LoadResult::fail(LoadErrorCode::FileNotFound,
            QString("LevelManager::buildBoardForLevel: nível %1 não existe no catálogo.")
                .arg(levelId));
        return nullptr;
    }

    if (meta->filePath.isEmpty())
    {
        outResult = LoadResult::fail(LoadErrorCode::FileNotFound,
            QString("LevelManager::buildBoardForLevel: nível %1 sem ficheiro associado.")
                .arg(levelId));
        return nullptr;
    }

    // Construir o Board via LevelLoader.
    std::unique_ptr<Board> board;
    LevelMetadata          updatedMeta;   // Recebe metadados atualizados do loader.

    outResult = LevelLoader::fromFile(meta->filePath, board, updatedMeta);

    if (!outResult.success)
    {
        qWarning().noquote()
            << "LevelManager::buildBoardForLevel: falha ao carregar nível"
            << levelId << "—" << outResult.errorMessage;
        return nullptr;
    }

    qDebug().noquote()
        << "LevelManager: Board do nível" << levelId << "construído com sucesso."
        << board->toString();

    return board;
}

// ─────────────────────────────────────────────────────────────────────────────
// Configuração
// ─────────────────────────────────────────────────────────────────────────────

void LevelManager::setCachingEnabled(bool enabled) noexcept
{
    cachingEnabled_ = enabled;
    qDebug().noquote()
        << "LevelManager: cache"
        << (cachingEnabled_ ? "activada." : "desactivada.");
}

void LevelManager::clearCache() noexcept
{
    // Na implementação atual, os metadados são a "cache" — limpar aqui
    // seria destruir o catálogo, o que não é o objetivo desta operação.
    // Reservado para quando a cache de Boards completos for implementada.
    qDebug() << "LevelManager::clearCache: (sem Boards em cache nesta versão).";
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString LevelManager::toString() const
{
    return QString("LevelManager[levels=%1 cachingEnabled=%2 firstId=%3 lastId=%4]")
        .arg(levelCount())
        .arg(cachingEnabled_ ? "true" : "false")
        .arg(firstLevelId())
        .arg(sortedIds_.isEmpty() ? -1 : sortedIds_.last());
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

void LevelManager::insertMetadata(const LevelMetadata& meta)
{
    metadataMap_.insert(meta.id, meta);

    // Inserção ordenada em sortedIds_ via busca binária — O(log n) na pesquisa
    // + O(n) na inserção no QList, aceitável para n ≤ algumas centenas de níveis.
    const auto it = std::lower_bound(sortedIds_.begin(),
                                     sortedIds_.end(),
                                     meta.id);

    // Se o id já estava presente (não devia acontecer — verificado antes),
    // não duplicar.
    if (it == sortedIds_.end() || *it != meta.id)
        sortedIds_.insert(it, meta.id);
}

void LevelManager::rebuildSortedIds()
{
    sortedIds_.clear();
    sortedIds_.reserve(metadataMap_.size());

    for (auto it = metadataMap_.cbegin(); it != metadataMap_.cend(); ++it)
        sortedIds_.append(it.key());

    std::sort(sortedIds_.begin(), sortedIds_.end());
}
