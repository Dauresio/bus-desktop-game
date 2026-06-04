/**
 * @file LevelLoader.cpp
 * @brief Implementação da classe LevelLoader para o jogo Bus Jam Puzzle.
 *
 * Fluxo de carregamento completo (fromFile / fromJson):
 *
 *   1. Leitura e parse do JSON (QJsonDocument).
 *   2. parseMeta()      → LevelMetadata + validação de dimensões.
 *   3. Criação do Board com as dimensões extraídas.
 *   4. parsePlatforms() → Platform objects adicionados ao Board.
 *   5. parseBuses()     → Bus objects adicionados ao Board.
 *   6. parsePassengers()→ Passenger objects adicionados ao Board.
 *   7. Board::validatePlatformCount() → verifica [4,8] plataformas.
 *
 * A ordem em 4→5→6 é intencional: as plataformas são adicionadas primeiro
 * para que o Board::occupancyGrid_ as registe antes de qualquer Bus ou
 * Passenger, o que permite detetar sobreposições corretamente.
 *
 * Política de erro:
 *  - Qualquer erro em qualquer fase aborta o carregamento e devolve
 *    LoadResult::fail() com código e mensagem específicos.
 *  - O Board parcialmente construído é descartado (unique_ptr não é
 *    preenchido).
 *  - Campos opcionais em falta adicionam avisos não fatais.
 */

#include "LevelLoader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// API pública: fromFile
// ─────────────────────────────────────────────────────────────────────────────

LoadResult LevelLoader::fromFile(const QString&          filePath,
                                 std::unique_ptr<Board>& outBoard,
                                 LevelMetadata&          outMeta)
{
    QFile file(filePath);

    if (!file.exists())
    {
        return LoadResult::fail(LoadErrorCode::FileNotFound,
            makeError(filePath, {}, "Ficheiro não encontrado."));
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return LoadResult::fail(LoadErrorCode::FileUnreadable,
            makeError(filePath, {}, "Sem permissão de leitura."));
    }

    const QByteArray rawData = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        return LoadResult::fail(LoadErrorCode::InvalidJson,
            makeError(filePath, {},
                QString("JSON inválido na posição %1: %2")
                    .arg(parseError.offset)
                    .arg(parseError.errorString())));
    }

    if (!doc.isObject())
    {
        return LoadResult::fail(LoadErrorCode::InvalidJson,
            makeError(filePath, {}, "O documento JSON deve ser um objeto raiz."));
    }

    LoadResult result = fromJson(doc.object(), outBoard, outMeta, filePath);

    if (result.success)
        outMeta.filePath = filePath;

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// API pública: fromJson
// ─────────────────────────────────────────────────────────────────────────────

LoadResult LevelLoader::fromJson(const QJsonObject&      json,
                                 std::unique_ptr<Board>& outBoard,
                                 LevelMetadata&          outMeta,
                                 const QString&          sourceHint)
{
    // ── Passo 1: metadados e dimensões ────────────────────────────────────
    {
        LoadResult r = parseMeta(json, outMeta, sourceHint);
        if (!r.success) return r;
    }

    // ── Passo 2: construir Board ──────────────────────────────────────────
    std::unique_ptr<Board> board;
    try
    {
        board = std::make_unique<Board>(outMeta.rows, outMeta.cols);
    }
    catch (const std::invalid_argument& ex)
    {
        return LoadResult::fail(LoadErrorCode::InvalidDimensions,
            makeError(sourceHint, {}, ex.what()));
    }

    // ── Passo 3: plataformas (antes de buses e passengers) ─────────────────
    {
        QJsonArray arr;
        LoadResult r = requireArray(json, QStringLiteral("platforms"), arr, sourceHint);
        if (!r.success) return r;

        r = parsePlatforms(arr, *board, sourceHint);
        if (!r.success) return r;

        outMeta.platformCount = arr.size();
    }

    // ── Passo 4: autocarros ───────────────────────────────────────────────
    {
        QJsonArray arr;
        LoadResult r = requireArray(json, QStringLiteral("buses"), arr, sourceHint);
        if (!r.success) return r;

        r = parseBuses(arr, *board, sourceHint);
        if (!r.success) return r;

        outMeta.busCount = arr.size();
    }

    // ── Passo 5: passageiros ──────────────────────────────────────────────
    {
        QJsonArray arr;
        LoadResult r = requireArray(json, QStringLiteral("passengers"), arr, sourceHint);
        if (!r.success) return r;

        r = parsePassengers(arr, *board, sourceHint);
        if (!r.success) return r;

        outMeta.passengerCount = arr.size();
    }

    // ── Passo 6: validação final do número de plataformas ─────────────────
    if (!board->validatePlatformCount())
    {
        return LoadResult::fail(LoadErrorCode::InvalidPlatformCount,
            makeError(sourceHint, {},
                QString("Número de plataformas inválido (%1). Intervalo: [%2, %3].")
                    .arg(board->platformCount())
                    .arg(BoardConstants::kMinPlatforms)
                    .arg(BoardConstants::kMaxPlatforms)));
    }

    // Transfere posse para o chamador.
    outBoard = std::move(board);

    LoadResult r = LoadResult::ok(outMeta.id);
    qDebug().noquote()
        << "LevelLoader: nível" << outMeta.id
        << QString("'%1'").arg(outMeta.name)
        << "carregado com sucesso."
        << "buses=" << outMeta.busCount
        << "passengers=" << outMeta.passengerCount
        << "platforms=" << outMeta.platformCount;

    return r;
}

// ─────────────────────────────────────────────────────────────────────────────
// API pública: metadataFromFile
// ─────────────────────────────────────────────────────────────────────────────

LoadResult LevelLoader::metadataFromFile(const QString& filePath,
                                         LevelMetadata& outMeta)
{
    QFile file(filePath);

    if (!file.exists())
        return LoadResult::fail(LoadErrorCode::FileNotFound,
            makeError(filePath, {}, "Ficheiro não encontrado."));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return LoadResult::fail(LoadErrorCode::FileUnreadable,
            makeError(filePath, {}, "Sem permissão de leitura."));

    const QByteArray rawData = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return LoadResult::fail(LoadErrorCode::InvalidJson,
            makeError(filePath, {}, "JSON inválido ou não é objeto raiz."));

    const QJsonObject json = doc.object();
    LoadResult r = parseMeta(json, outMeta, filePath);

    if (r.success)
    {
        // Contar entidades sem as construir.
        outMeta.busCount       = json.value(QStringLiteral("buses")).toArray().size();
        outMeta.passengerCount = json.value(QStringLiteral("passengers")).toArray().size();
        outMeta.platformCount  = json.value(QStringLiteral("platforms")).toArray().size();
        outMeta.filePath       = filePath;
    }

    return r;
}

// ─────────────────────────────────────────────────────────────────────────────
// API pública: toJson
// ─────────────────────────────────────────────────────────────────────────────

QJsonObject LevelLoader::toJson(const Board& board, const LevelMetadata& meta)
{
    QJsonObject root;

    // Versão do esquema.
    root[QStringLiteral("schemaVersion")] = LevelLoaderConstants::kCurrentSchemaVersion;

    // Metadados.
    root[QStringLiteral("id")]            = meta.id;
    root[QStringLiteral("name")]          = meta.name;
    root[QStringLiteral("difficulty")]    = meta.difficulty;
    root[QStringLiteral("rows")]          = board.rows();
    root[QStringLiteral("cols")]          = board.cols();
    root[QStringLiteral("timeLimitSecs")] = meta.timeLimitSecs;

    // Autocarros.
    QJsonArray busArray;
    for (const auto& busPtr : board.buses())
    {
        const Bus* bus = busPtr.get();
        QJsonObject busObj;
        busObj[QStringLiteral("id")]        = bus->id();
        busObj[QStringLiteral("color")]     = colorToString(bus->color());
        busObj[QStringLiteral("capacity")]  = bus->capacity();
        busObj[QStringLiteral("row")]       = bus->row();
        busObj[QStringLiteral("col")]       = bus->col();
        busObj[QStringLiteral("direction")] = directionToString(bus->direction());
        busArray.append(busObj);
    }
    root[QStringLiteral("buses")] = busArray;

    // Passageiros.
    QJsonArray passengerArray;
    for (const auto& pPtr : board.passengers())
    {
        const Passenger* p = pPtr.get();
        QJsonObject pObj;
        pObj[QStringLiteral("id")]    = p->id();
        pObj[QStringLiteral("color")] = colorToString(p->color());
        pObj[QStringLiteral("row")]   = p->row();
        pObj[QStringLiteral("col")]   = p->col();
        passengerArray.append(pObj);
    }
    root[QStringLiteral("passengers")] = passengerArray;

    // Plataformas.
    QJsonArray platformArray;
    for (const auto& plPtr : board.platforms())
    {
        const Platform* pl = plPtr.get();
        QJsonObject plObj;
        plObj[QStringLiteral("id")]            = pl->id();
        plObj[QStringLiteral("color")]         = colorToString(pl->color());
        plObj[QStringLiteral("row")]           = pl->row();
        plObj[QStringLiteral("col")]           = pl->col();
        plObj[QStringLiteral("exitDirection")] = directionToString(pl->exitDirection());
        platformArray.append(plObj);
    }
    root[QStringLiteral("platforms")] = platformArray;

    return root;
}

// ─────────────────────────────────────────────────────────────────────────────
// API pública: toFile
// ─────────────────────────────────────────────────────────────────────────────

bool LevelLoader::toFile(const QString&       filePath,
                          const Board&         board,
                          const LevelMetadata& meta)
{
    const QJsonObject json = toJson(board, meta);
    const QJsonDocument doc(json);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qWarning().noquote()
            << "LevelLoader::toFile: não foi possível abrir para escrita:"
            << filePath;
        return false;
    }

    const qint64 written = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    if (written < 0)
    {
        qWarning().noquote()
            << "LevelLoader::toFile: erro de escrita em" << filePath;
        return false;
    }

    qDebug().noquote()
        << "LevelLoader::toFile: nível" << meta.id
        << "gravado em" << filePath
        << "(" << written << "bytes).";

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// API pública: validate
// ─────────────────────────────────────────────────────────────────────────────

LoadResult LevelLoader::validate(const QJsonObject& json, QStringList& outWarnings)
{
    outWarnings.clear();

    // Verificar versão do esquema (opcional — aviso se ausente).
    if (!json.contains(QStringLiteral("schemaVersion")))
    {
        outWarnings << QStringLiteral("Campo 'schemaVersion' ausente; assumido v1.");
    }
    else
    {
        const int version = json.value(QStringLiteral("schemaVersion")).toInt(-1);
        if (version != LevelLoaderConstants::kCurrentSchemaVersion)
        {
            return LoadResult::fail(LoadErrorCode::SchemaVersion,
                makeError({}, {}, QString("Versão de esquema %1 não suportada. "
                    "Versão atual: %2.")
                    .arg(version)
                    .arg(LevelLoaderConstants::kCurrentSchemaVersion)));
        }
    }

    // Verificar campos obrigatórios de topo.
    const QStringList requiredFields = {
        QStringLiteral("id"),
        QStringLiteral("rows"),
        QStringLiteral("cols"),
        QStringLiteral("buses"),
        QStringLiteral("passengers"),
        QStringLiteral("platforms")
    };

    for (const QString& field : requiredFields)
    {
        if (!json.contains(field))
        {
            return LoadResult::fail(LoadErrorCode::MissingField,
                makeError({}, {}, QString("Campo obrigatório '%1' ausente.").arg(field)));
        }
    }

    // Campos opcionais.
    if (!json.contains(QStringLiteral("name")))
        outWarnings << QStringLiteral("Campo 'name' ausente; será usado id como nome.");

    if (!json.contains(QStringLiteral("difficulty")))
        outWarnings << QStringLiteral("Campo 'difficulty' ausente; assumido 1.");

    if (!json.contains(QStringLiteral("timeLimitSecs")))
        outWarnings << QStringLiteral("Campo 'timeLimitSecs' ausente; assumido 0 (sem limite).");

    return LoadResult::ok(json.value(QStringLiteral("id")).toInt(-1));
}

// ─────────────────────────────────────────────────────────────────────────────
// Parsers internos
// ─────────────────────────────────────────────────────────────────────────────

LoadResult LevelLoader::parseMeta(const QJsonObject& json,
                                   LevelMetadata&     outMeta,
                                   const QString&     sourceHint)
{
    // id (obrigatório)
    {
        LoadResult r = requireInt(json, QStringLiteral("id"), outMeta.id, sourceHint);
        if (!r.success) return r;
        if (outMeta.id < 1)
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, "id", "Deve ser >= 1."));
    }

    // rows (obrigatório)
    {
        LoadResult r = requireInt(json, QStringLiteral("rows"), outMeta.rows, sourceHint);
        if (!r.success) return r;
    }

    // cols (obrigatório)
    {
        LoadResult r = requireInt(json, QStringLiteral("cols"), outMeta.cols, sourceHint);
        if (!r.success) return r;
    }

    // Dimensões — validadas aqui antes de criar o Board.
    if (outMeta.rows < BoardConstants::kMinRows || outMeta.rows > BoardConstants::kMaxRows)
        return LoadResult::fail(LoadErrorCode::InvalidDimensions,
            makeError(sourceHint, "rows",
                QString("Valor %1 fora do intervalo [%2, %3].")
                    .arg(outMeta.rows)
                    .arg(BoardConstants::kMinRows)
                    .arg(BoardConstants::kMaxRows)));

    if (outMeta.cols < BoardConstants::kMinCols || outMeta.cols > BoardConstants::kMaxCols)
        return LoadResult::fail(LoadErrorCode::InvalidDimensions,
            makeError(sourceHint, "cols",
                QString("Valor %1 fora do intervalo [%2, %3].")
                    .arg(outMeta.cols)
                    .arg(BoardConstants::kMinCols)
                    .arg(BoardConstants::kMaxCols)));

    // name (opcional)
    if (json.contains(QStringLiteral("name")))
    {
        outMeta.name = json.value(QStringLiteral("name")).toString().trimmed();
        if (outMeta.name.length() > LevelLoaderConstants::kMaxLevelNameLength)
            outMeta.name.truncate(LevelLoaderConstants::kMaxLevelNameLength);
    }
    else
    {
        outMeta.name = QString("Level %1").arg(outMeta.id);
    }

    // difficulty (opcional, padrão 1)
    outMeta.difficulty = json.value(QStringLiteral("difficulty")).toInt(1);
    if (outMeta.difficulty < LevelLoaderConstants::kMinDifficulty
     || outMeta.difficulty > LevelLoaderConstants::kMaxDifficulty)
    {
        outMeta.difficulty = 1;
        qWarning().noquote()
            << makeError(sourceHint, "difficulty",
                "Valor fora de [1,5]; assumido 1.");
    }

    // timeLimitSecs (opcional, padrão 0)
    outMeta.timeLimitSecs = json.value(QStringLiteral("timeLimitSecs")).toInt(0);
    if (outMeta.timeLimitSecs < 0)
        outMeta.timeLimitSecs = 0;

    return LoadResult::ok(outMeta.id);
}

LoadResult LevelLoader::parseBuses(const QJsonArray& busArray,
                                    Board&            board,
                                    const QString&    sourceHint)
{
    if (busArray.isEmpty())
        return LoadResult::fail(LoadErrorCode::MissingField,
            makeError(sourceHint, "buses", "Array de autocarros vazio."));

    for (int i = 0; i < busArray.size(); ++i)
    {
        const QString ctx = QString("buses[%1]").arg(i);

        if (!busArray[i].isObject())
            return LoadResult::fail(LoadErrorCode::InvalidJson,
                makeError(sourceHint, ctx, "Elemento não é um objeto JSON."));

        const QJsonObject obj = busArray[i].toObject();

        int     id, capacity, row, col;
        QString colorStr, dirStr;

        // Campos obrigatórios.
        auto r = requireInt(obj, QStringLiteral("id"), id, sourceHint, ctx);
        if (!r.success) return r;

        r = requireString(obj, QStringLiteral("color"), colorStr, sourceHint, ctx);
        if (!r.success) return r;

        r = requireInt(obj, QStringLiteral("capacity"), capacity, sourceHint, ctx);
        if (!r.success) return r;

        r = requireInt(obj, QStringLiteral("row"), row, sourceHint, ctx);
        if (!r.success) return r;

        r = requireInt(obj, QStringLiteral("col"), col, sourceHint, ctx);
        if (!r.success) return r;

        r = requireString(obj, QStringLiteral("direction"), dirStr, sourceHint, ctx);
        if (!r.success) return r;

        // Conversões de enum.
        bool colorOk = false, dirOk = false;
        const BusColor  color = colorFromString(colorStr, colorOk);
        const Direction dir   = directionFromString(dirStr, dirOk);

        if (!colorOk)
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, ctx + ".color",
                    QString("Cor '%1' não reconhecida.").arg(colorStr)));

        if (!dirOk)
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, ctx + ".direction",
                    QString("Direção '%1' não reconhecida.").arg(dirStr)));

        // Validar capacidade.
        if (!Bus::isValidCapacity(capacity))
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, ctx + ".capacity",
                    QString("Capacidade %1 inválida. Valores: 4, 6, 8, 12.").arg(capacity)));

        // Construir Bus e adicionar ao Board.
        try
        {
            auto bus = std::make_unique<Bus>(id, color, capacity,
                                             QPoint(col, row), dir);

            if (!board.addBus(std::move(bus)))
            {
                return LoadResult::fail(LoadErrorCode::EntityOverlap,
                    makeError(sourceHint, ctx,
                        QString("Bus id=%1 em (%2,%3) sobrepõe entidade existente.")
                            .arg(id).arg(row).arg(col)));
            }
        }
        catch (const std::invalid_argument& ex)
        {
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, ctx, ex.what()));
        }
    }

    return LoadResult::ok(-1);
}

LoadResult LevelLoader::parsePassengers(const QJsonArray& passengerArray,
                                         Board&            board,
                                         const QString&    sourceHint)
{
    if (passengerArray.isEmpty())
        return LoadResult::fail(LoadErrorCode::MissingField,
            makeError(sourceHint, "passengers", "Array de passageiros vazio."));

    for (int i = 0; i < passengerArray.size(); ++i)
    {
        const QString ctx = QString("passengers[%1]").arg(i);

        if (!passengerArray[i].isObject())
            return LoadResult::fail(LoadErrorCode::InvalidJson,
                makeError(sourceHint, ctx, "Elemento não é um objeto JSON."));

        const QJsonObject obj = passengerArray[i].toObject();

        int     id, row, col;
        QString colorStr;

        auto r = requireInt(obj, QStringLiteral("id"), id, sourceHint, ctx);
        if (!r.success) return r;

        r = requireString(obj, QStringLiteral("color"), colorStr, sourceHint, ctx);
        if (!r.success) return r;

        r = requireInt(obj, QStringLiteral("row"), row, sourceHint, ctx);
        if (!r.success) return r;

        r = requireInt(obj, QStringLiteral("col"), col, sourceHint, ctx);
        if (!r.success) return r;

        bool colorOk = false;
        const BusColor color = colorFromString(colorStr, colorOk);

        if (!colorOk)
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, ctx + ".color",
                    QString("Cor '%1' não reconhecida.").arg(colorStr)));

        auto passenger = std::make_unique<Passenger>(id, color, QPoint(col, row));

        if (!board.addPassenger(std::move(passenger)))
        {
            return LoadResult::fail(LoadErrorCode::EntityOverlap,
                makeError(sourceHint, ctx,
                    QString("Passenger id=%1 em (%2,%3) sobrepõe entidade existente.")
                        .arg(id).arg(row).arg(col)));
        }
    }

    return LoadResult::ok(-1);
}

LoadResult LevelLoader::parsePlatforms(const QJsonArray& platformArray,
                                        Board&            board,
                                        const QString&    sourceHint)
{
    if (platformArray.isEmpty())
        return LoadResult::fail(LoadErrorCode::MissingField,
            makeError(sourceHint, "platforms", "Array de plataformas vazio."));

    for (int i = 0; i < platformArray.size(); ++i)
    {
        const QString ctx = QString("platforms[%1]").arg(i);

        if (!platformArray[i].isObject())
            return LoadResult::fail(LoadErrorCode::InvalidJson,
                makeError(sourceHint, ctx, "Elemento não é um objeto JSON."));

        const QJsonObject obj = platformArray[i].toObject();

        int     id, row, col;
        QString colorStr, exitDirStr;

        auto r = requireInt(obj, QStringLiteral("id"), id, sourceHint, ctx);
        if (!r.success) return r;

        r = requireString(obj, QStringLiteral("color"), colorStr, sourceHint, ctx);
        if (!r.success) return r;

        r = requireInt(obj, QStringLiteral("row"), row, sourceHint, ctx);
        if (!r.success) return r;

        r = requireInt(obj, QStringLiteral("col"), col, sourceHint, ctx);
        if (!r.success) return r;

        r = requireString(obj, QStringLiteral("exitDirection"), exitDirStr, sourceHint, ctx);
        if (!r.success) return r;

        bool colorOk = false, dirOk = false;
        const BusColor  color   = colorFromString(colorStr, colorOk);
        const Direction exitDir = directionFromString(exitDirStr, dirOk);

        if (!colorOk)
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, ctx + ".color",
                    QString("Cor '%1' não reconhecida.").arg(colorStr)));

        if (!dirOk)
            return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
                makeError(sourceHint, ctx + ".exitDirection",
                    QString("Direção '%1' não reconhecida.").arg(exitDirStr)));

        auto platform = std::make_unique<Platform>(id, color, QPoint(col, row), exitDir);

        if (!board.addPlatform(std::move(platform)))
        {
            return LoadResult::fail(LoadErrorCode::EntityOverlap,
                makeError(sourceHint, ctx,
                    QString("Platform id=%1 em (%2,%3) sobrepõe entidade existente.")
                        .arg(id).arg(row).arg(col)));
        }
    }

    return LoadResult::ok(-1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Conversores enum ↔ string
// ─────────────────────────────────────────────────────────────────────────────

BusColor LevelLoader::colorFromString(const QString& s, bool& ok) noexcept
{
    ok = true;
    const QString lower = s.trimmed().toLower();

    if (lower == QLatin1String("red"))    return BusColor::Red;
    if (lower == QLatin1String("blue"))   return BusColor::Blue;
    if (lower == QLatin1String("green"))  return BusColor::Green;
    if (lower == QLatin1String("yellow")) return BusColor::Yellow;
    if (lower == QLatin1String("purple")) return BusColor::Purple;

    ok = false;
    return BusColor::Red;
}

Direction LevelLoader::directionFromString(const QString& s, bool& ok) noexcept
{
    ok = true;
    const QString lower = s.trimmed().toLower();

    if (lower == QLatin1String("up"))    return Direction::Up;
    if (lower == QLatin1String("down"))  return Direction::Down;
    if (lower == QLatin1String("left"))  return Direction::Left;
    if (lower == QLatin1String("right")) return Direction::Right;

    ok = false;
    return Direction::Right;
}

QString LevelLoader::colorToString(BusColor color) noexcept
{
    switch (color)
    {
        case BusColor::Red:    return QStringLiteral("Red");
        case BusColor::Blue:   return QStringLiteral("Blue");
        case BusColor::Green:  return QStringLiteral("Green");
        case BusColor::Yellow: return QStringLiteral("Yellow");
        case BusColor::Purple: return QStringLiteral("Purple");
    }
    Q_UNREACHABLE();
    return {};
}

QString LevelLoader::directionToString(Direction dir) noexcept
{
    switch (dir)
    {
        case Direction::Up:    return QStringLiteral("Up");
        case Direction::Down:  return QStringLiteral("Down");
        case Direction::Left:  return QStringLiteral("Left");
        case Direction::Right: return QStringLiteral("Right");
    }
    Q_UNREACHABLE();
    return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares de extração de campos JSON
// ─────────────────────────────────────────────────────────────────────────────

LoadResult LevelLoader::requireInt(const QJsonObject& obj,
                                    const QString&     key,
                                    int&               outVal,
                                    const QString&     sourceHint,
                                    const QString&     context)
{
    if (!obj.contains(key))
        return LoadResult::fail(LoadErrorCode::MissingField,
            makeError(sourceHint, context,
                QString("Campo obrigatório '%1' ausente.").arg(key)));

    const QJsonValue val = obj.value(key);
    if (!val.isDouble())
        return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
            makeError(sourceHint, context,
                QString("Campo '%1' deve ser um inteiro.").arg(key)));

    outVal = val.toInt();
    return LoadResult::ok(-1);
}

LoadResult LevelLoader::requireString(const QJsonObject& obj,
                                       const QString&     key,
                                       QString&           outVal,
                                       const QString&     sourceHint,
                                       const QString&     context)
{
    if (!obj.contains(key))
        return LoadResult::fail(LoadErrorCode::MissingField,
            makeError(sourceHint, context,
                QString("Campo obrigatório '%1' ausente.").arg(key)));

    const QJsonValue val = obj.value(key);
    if (!val.isString())
        return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
            makeError(sourceHint, context,
                QString("Campo '%1' deve ser uma string.").arg(key)));

    outVal = val.toString();
    return LoadResult::ok(-1);
}

LoadResult LevelLoader::requireArray(const QJsonObject& obj,
                                      const QString&     key,
                                      QJsonArray&        outArr,
                                      const QString&     sourceHint,
                                      const QString&     context)
{
    if (!obj.contains(key))
        return LoadResult::fail(LoadErrorCode::MissingField,
            makeError(sourceHint, context,
                QString("Campo obrigatório '%1' ausente.").arg(key)));

    const QJsonValue val = obj.value(key);
    if (!val.isArray())
        return LoadResult::fail(LoadErrorCode::InvalidFieldValue,
            makeError(sourceHint, context,
                QString("Campo '%1' deve ser um array.").arg(key)));

    outArr = val.toArray();
    return LoadResult::ok(-1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliar de formatação de mensagens
// ─────────────────────────────────────────────────────────────────────────────

QString LevelLoader::makeError(const QString& sourceHint,
                                const QString& context,
                                const QString& message)
{
    QString result;

    if (!sourceHint.isEmpty())
        result += QStringLiteral("[%1] ").arg(sourceHint);

    if (!context.isEmpty())
        result += QStringLiteral("%1: ").arg(context);

    result += message;
    return result;
}
