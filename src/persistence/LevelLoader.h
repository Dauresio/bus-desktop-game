/**
 * @file LevelLoader.h
 * @brief Declaração da classe LevelLoader para o jogo Bus Jam Puzzle.
 *
 * LevelLoader é responsável por toda a lógica de serialização e
 * deserialização entre o formato JSON em disco e os objetos de domínio
 * (Board, Bus, Passenger, Platform).
 *
 * ### Responsabilidades:
 *  - Ler ficheiros JSON de nível do sistema de ficheiros ou recursos Qt.
 *  - Validar o esquema e os valores antes de criar qualquer objeto.
 *  - Construir um Board completamente populado e validado.
 *  - Serializar o estado atual de um Board de volta a JSON (save state).
 *  - Reportar erros de forma estruturada via LoadResult.
 *
 * ### O que LevelLoader NÃO faz:
 *  - Não gere coleções de níveis (responsabilidade do LevelManager).
 *  - Não persiste progresso do jogador (responsabilidade do PersistenceManager).
 *  - Não conhece QML nem o ciclo de vida do jogo.
 *
 * ### Formato JSON de um nível:
 * @code
 * {
 *   "id": 1,
 *   "name": "Primeiro Nível",
 *   "difficulty": 1,
 *   "rows": 8,
 *   "cols": 8,
 *   "timeLimitSecs": 0,
 *   "buses": [
 *     {
 *       "id": 0,
 *       "color": "Red",
 *       "capacity": 4,
 *       "row": 3,
 *       "col": 0,
 *       "direction": "Right"
 *     }
 *   ],
 *   "passengers": [
 *     { "id": 0, "color": "Red", "row": 3, "col": 3 }
 *   ],
 *   "platforms": [
 *     {
 *       "id": 0,
 *       "color": "Red",
 *       "row": 3,
 *       "col": 7,
 *       "exitDirection": "Right"
 *     }
 *   ]
 * }
 * @endcode
 *
 * ### Política de validação:
 *  - Campos obrigatórios em falta → erro (nível não carregado).
 *  - Valores fora de intervalo (ex.: capacidade 5) → erro.
 *  - Sobreposição de entidades → erro.
 *  - Campos opcionais em falta → valor por omissão documentado.
 *
 * ### Recursos Qt vs sistema de ficheiros:
 *  O método fromFile() aceita caminhos absolutos, relativos e caminhos
 *  de recursos Qt (":/levels/level_001.json"). O acesso é transparente
 *  via QFile.
 */

#pragma once

#include "Board.h"

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <memory>

// ─────────────────────────────────────────────────────────────────────────────
// Estruturas de resultado
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Código de erro de carregamento de nível.
 */
enum class LoadErrorCode : quint8
{
    None               = 0,   ///< Sem erro.
    FileNotFound       = 1,   ///< Ficheiro não encontrado.
    FileUnreadable     = 2,   ///< Sem permissão de leitura.
    InvalidJson        = 3,   ///< JSON malformado.
    MissingField       = 4,   ///< Campo obrigatório em falta.
    InvalidFieldValue  = 5,   ///< Valor fora do intervalo permitido.
    DuplicateId        = 6,   ///< Id de entidade duplicado.
    EntityOverlap      = 7,   ///< Duas entidades na mesma célula.
    InvalidDimensions  = 8,   ///< Dimensões do tabuleiro inválidas.
    InvalidPlatformCount = 9, ///< Número de plataformas fora de [4,8].
    SchemaVersion      = 10   ///< Versão de esquema incompatível.
};

/**
 * @brief Resultado estruturado de uma operação de carregamento.
 *
 * Permite ao LevelManager e ao GameController distinguir entre tipos de
 * falha e apresentar mensagens adequadas ao utilizador ou nos logs.
 */
struct LoadResult
{
    bool          success  { false };
    LoadErrorCode errorCode{ LoadErrorCode::None };
    QString       errorMessage;        ///< Mensagem legível para logs/UI.
    int           levelId  { -1 };     ///< Id do nível carregado (se sucesso).
    QStringList   warnings;            ///< Avisos não fatais (campos opcionais).

    /** @brief Constrói um resultado de sucesso. */
    [[nodiscard]] static LoadResult ok(int id)
    {
        LoadResult r;
        r.success = true;
        r.levelId = id;
        return r;
    }

    /** @brief Constrói um resultado de falha. */
    [[nodiscard]] static LoadResult fail(LoadErrorCode code, const QString& msg)
    {
        LoadResult r;
        r.success      = false;
        r.errorCode    = code;
        r.errorMessage = msg;
        return r;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Metadados de nível (sem criar o Board completo)
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Metadados leves de um nível, sem a estrutura completa do tabuleiro.
 *
 * Usado pelo LevelManager para listar níveis disponíveis sem carregar
 * todos os Boards em memória.
 */
struct LevelMetadata
{
    int     id            { -1 };
    QString name;
    int     difficulty    { 1 };      ///< 1–5.
    int     rows          { 8 };
    int     cols          { 8 };
    int     timeLimitSecs { 0 };      ///< 0 = sem limite.
    int     busCount      { 0 };
    int     passengerCount{ 0 };
    int     platformCount { 0 };
    QString filePath;                 ///< Caminho de onde foi carregado.
};

// ─────────────────────────────────────────────────────────────────────────────
// Constantes do módulo
// ─────────────────────────────────────────────────────────────────────────────

namespace LevelLoaderConstants
{
    constexpr int kCurrentSchemaVersion = 1;
    constexpr int kMaxLevelNameLength   = 64;
    constexpr int kMaxDifficulty        = 5;
    constexpr int kMinDifficulty        = 1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Classe LevelLoader
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Serializador/deserializador de níveis JSON para o Bus Jam Puzzle.
 *
 * Todos os métodos são estáticos — LevelLoader é um serviço sem estado.
 * Não é necessário instanciar; use diretamente via LevelLoader::fromFile().
 */
class LevelLoader
{
public:
    // LevelLoader não é instanciável.
    LevelLoader()  = delete;
    ~LevelLoader() = delete;

    // ─── API principal ────────────────────────────────────────────────────

    /**
     * @brief Carrega um nível completo a partir de um ficheiro JSON.
     *
     * Lê o ficheiro, valida o esquema, constrói e popula um Board.
     *
     * @param filePath Caminho para o ficheiro JSON (absoluto, relativo ou
     *                 recurso Qt ":/levels/...").
     * @param outBoard [out] unique_ptr populado com o Board completo.
     *                 Indefinido se result.success == false.
     * @param outMeta  [out] Metadados do nível carregado.
     *                 Indefinido se result.success == false.
     * @return LoadResult com sucesso ou código de erro detalhado.
     */
    [[nodiscard]] static LoadResult fromFile(const QString&          filePath,
                                             std::unique_ptr<Board>& outBoard,
                                             LevelMetadata&          outMeta);

    /**
     * @brief Carrega um nível a partir de um QJsonObject já em memória.
     *
     * Útil para testes unitários e para níveis gerados programaticamente.
     *
     * @param json     Objeto JSON com a definição do nível.
     * @param outBoard [out] Board populado.
     * @param outMeta  [out] Metadados.
     * @param sourceHint Identificador opcional para mensagens de erro.
     * @return LoadResult.
     */
    [[nodiscard]] static LoadResult fromJson(const QJsonObject&      json,
                                             std::unique_ptr<Board>& outBoard,
                                             LevelMetadata&          outMeta,
                                             const QString&          sourceHint = {});

    /**
     * @brief Lê apenas os metadados de um ficheiro, sem construir o Board.
     *
     * Operação leve: útil para popular a lista de níveis no LevelManager
     * sem carregar todas as entidades.
     *
     * @param filePath Caminho para o ficheiro JSON.
     * @param outMeta  [out] Metadados preenchidos.
     * @return LoadResult (success=true se metadados lidos com sucesso).
     */
    [[nodiscard]] static LoadResult metadataFromFile(const QString&  filePath,
                                                     LevelMetadata&  outMeta);

    /**
     * @brief Serializa o estado atual de um Board para JSON.
     *
     * Captura a configuração inicial das entidades (posições de início,
     * cores, capacidades) — não o estado mutável (jogadas feitas).
     * Usa-se para salvar a definição de um nível editado.
     *
     * @param board    Board a serializar.
     * @param meta     Metadados a incluir no JSON (nome, dificuldade, etc.).
     * @return QJsonObject com a definição completa.
     */
    [[nodiscard]] static QJsonObject toJson(const Board&         board,
                                            const LevelMetadata& meta);

    /**
     * @brief Grava um Board para ficheiro JSON.
     *
     * @param filePath Caminho de destino.
     * @param board    Board a gravar.
     * @param meta     Metadados a incluir.
     * @return true se escrita bem-sucedida; false caso contrário.
     */
    [[nodiscard]] static bool toFile(const QString&       filePath,
                                     const Board&         board,
                                     const LevelMetadata& meta);

    /**
     * @brief Valida um QJsonObject sem construir o Board.
     *
     * Útil para um eventual editor de níveis que quer validar
     * em tempo real sem instanciar o Board completo.
     *
     * @param json       Objeto a validar.
     * @param outWarnings [out] Avisos não fatais (campos opcionais em falta).
     * @return LoadResult com success=true se válido.
     */
    [[nodiscard]] static LoadResult validate(const QJsonObject& json,
                                             QStringList&       outWarnings);

private:
    // ─── Parsers internos por secção ──────────────────────────────────────

    /**
     * @brief Valida e extrai os metadados do nível (id, name, dims, etc.).
     *
     * @param json       JSON raiz do nível.
     * @param outMeta    [out] Metadados extraídos.
     * @param sourceHint Identificador para mensagens de erro.
     * @return LoadResult.
     */
    [[nodiscard]] static LoadResult parseMeta(const QJsonObject& json,
                                              LevelMetadata&     outMeta,
                                              const QString&     sourceHint);

    /**
     * @brief Constrói e adiciona todos os autocarros ao Board.
     *
     * @param busArray   Array JSON "buses".
     * @param board      Board destino.
     * @param sourceHint Identificador para mensagens de erro.
     * @return LoadResult.
     */
    [[nodiscard]] static LoadResult parseBuses(const QJsonArray& busArray,
                                               Board&            board,
                                               const QString&    sourceHint);

    /**
     * @brief Constrói e adiciona todos os passageiros ao Board.
     *
     * @param passengerArray Array JSON "passengers".
     * @param board          Board destino.
     * @param sourceHint     Identificador para mensagens de erro.
     * @return LoadResult.
     */
    [[nodiscard]] static LoadResult parsePassengers(const QJsonArray& passengerArray,
                                                    Board&            board,
                                                    const QString&    sourceHint);

    /**
     * @brief Constrói e adiciona todas as plataformas ao Board.
     *
     * @param platformArray Array JSON "platforms".
     * @param board         Board destino.
     * @param sourceHint    Identificador para mensagens de erro.
     * @return LoadResult.
     */
    [[nodiscard]] static LoadResult parsePlatforms(const QJsonArray& platformArray,
                                                   Board&            board,
                                                   const QString&    sourceHint);

    // ─── Conversores de string ↔ enum ─────────────────────────────────────

    /**
     * @brief Converte string para BusColor.
     *
     * @param s String a converter ("Red", "Blue", etc.).
     * @param ok [out] false se a string não for reconhecida.
     * @return BusColor correspondente; BusColor::Red se !ok.
     */
    [[nodiscard]] static BusColor  colorFromString(const QString& s, bool& ok) noexcept;

    /**
     * @brief Converte string para Direction.
     *
     * @param s String a converter ("Up", "Down", "Left", "Right").
     * @param ok [out] false se a string não for reconhecida.
     * @return Direction correspondente; Direction::Right se !ok.
     */
    [[nodiscard]] static Direction directionFromString(const QString& s, bool& ok) noexcept;

    /**
     * @brief Converte BusColor para string legível.
     */
    [[nodiscard]] static QString colorToString(BusColor color) noexcept;

    /**
     * @brief Converte Direction para string legível.
     */
    [[nodiscard]] static QString directionToString(Direction dir) noexcept;

    // ─── Auxiliares de extração de campos JSON ────────────────────────────

    /**
     * @brief Extrai um inteiro obrigatório de um QJsonObject.
     *
     * @param obj        Objeto JSON.
     * @param key        Chave a extrair.
     * @param outVal     [out] Valor extraído.
     * @param sourceHint Identificador para mensagens de erro.
     * @param context    Contexto adicional (ex.: "buses[0]").
     * @return LoadResult (success=true se campo existe e é inteiro).
     */
    [[nodiscard]] static LoadResult requireInt(const QJsonObject& obj,
                                               const QString&     key,
                                               int&               outVal,
                                               const QString&     sourceHint,
                                               const QString&     context = {});

    /**
     * @brief Extrai uma string obrigatória de um QJsonObject.
     */
    [[nodiscard]] static LoadResult requireString(const QJsonObject& obj,
                                                  const QString&     key,
                                                  QString&           outVal,
                                                  const QString&     sourceHint,
                                                  const QString&     context = {});

    /**
     * @brief Extrai um array obrigatório de um QJsonObject.
     */
    [[nodiscard]] static LoadResult requireArray(const QJsonObject& obj,
                                                 const QString&     key,
                                                 QJsonArray&        outArr,
                                                 const QString&     sourceHint,
                                                 const QString&     context = {});

    /**
     * @brief Constrói uma mensagem de erro padronizada.
     *
     * Formato: "[sourceHint] context: mensagem"
     */
    [[nodiscard]] static QString makeError(const QString& sourceHint,
                                           const QString& context,
                                           const QString& message);
};
