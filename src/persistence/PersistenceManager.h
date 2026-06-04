/**
 * @file PersistenceManager.h
 * @brief Declaração da classe PersistenceManager para o jogo Bus Jam Puzzle.
 *
 * PersistenceManager centraliza toda a leitura e escrita de dados persistentes:
 * progresso do jogador, melhores pontuações, definições de jogo e estado de
 * sessão (nível em curso ao fechar a aplicação).
 *
 * ### Backends de armazenamento:
 *  - QSettings  → chave/valor simples (progresso, definições, flags).
 *  - QJsonDocument → estruturas complexas (histórico de resultados por nível).
 *    Guardado num ficheiro JSON no diretório de dados da aplicação.
 *
 * ### Separação de responsabilidades:
 *  PersistenceManager lida com I/O e serialização.
 *  GameController decide *quando* guardar e *o que* guardar.
 *  LevelManager fornece os ids de nível — PersistenceManager não os valida.
 *
 * ### Grupos de dados:
 *
 *  1. **Progresso**
 *     - Nível mais alto desbloqueado.
 *     - Nível em que o jogador estava ao fechar.
 *
 *  2. **Resultados por nível** (best scores)
 *     - Melhor pontuação.
 *     - Melhor número de jogadas.
 *     - Melhor tempo.
 *     - Número de estrelas (0–3).
 *     - Número de vezes que o nível foi completado.
 *
 *  3. **Definições**
 *     - Som activado/desactivado.
 *     - Música activada/desactivada.
 *     - Idioma preferido.
 *     - Esquema de cores (tema).
 *
 *  4. **Estado de sessão**
 *     - Snapshot JSON do nível em curso (para retomar após fecho).
 *     - Jogadas feitas na sessão actual.
 *     - Tempo decorrido na sessão actual.
 *
 * ### Thread-safety:
 *  Não é thread-safe. Todas as chamadas devem ocorrer na thread principal.
 *  QSettings é thread-safe internamente, mas os métodos desta classe
 *  não são reentrantes.
 *
 * ### Migração de versão:
 *  A versão do esquema de dados é guardada em QSettings. Se a versão
 *  no disco for inferior à versão atual, migrateIfNeeded() é chamado
 *  automaticamente no construtor.
 */

#pragma once

#include <QString>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

// ─────────────────────────────────────────────────────────────────────────────
// Estruturas de dados persistidos
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Resultado guardado para um nível específico.
 *
 * Guarda apenas os melhores valores — não o histórico completo de tentativas.
 * O número de vezes completado é excepção: acumula sempre.
 */
struct SavedLevelResult
{
    int  levelId          { -1 };
    int  bestScore        { 0 };
    int  bestMoves        { INT_MAX };  ///< Menor número de jogadas.
    int  bestTimeSecs     { INT_MAX };  ///< Menor tempo em segundos.
    int  stars            { 0 };        ///< 0–3 estrelas.
    int  completionCount  { 0 };        ///< Vezes que o nível foi completado.
    QDateTime lastPlayed;               ///< Data/hora da última tentativa.

    /** @brief true se este registo tem dados reais (foi alguma vez completado). */
    [[nodiscard]] bool isValid() const noexcept { return levelId >= 1; }
};

/**
 * @brief Definições persistidas do jogador.
 */
struct GameSettings
{
    bool    soundEnabled  { true };
    bool    musicEnabled  { true };
    QString language      { QStringLiteral("pt") };
    QString colorTheme    { QStringLiteral("default") };
    int     masterVolume  { 100 };   ///< 0–100.
};

/**
 * @brief Estado da sessão em curso (para retomar após fecho abrupto).
 */
struct SessionState
{
    int  levelId      { -1 };
    int  moveCount    { 0 };
    int  elapsedSecs  { 0 };
    bool isActive     { false };  ///< true se há uma sessão a guardar.
};

// ─────────────────────────────────────────────────────────────────────────────
// Constantes
// ─────────────────────────────────────────────────────────────────────────────

namespace PersistenceConstants
{
    constexpr int kCurrentDataVersion  = 1;
    constexpr int kMaxTrackedLevels    = 500;

    // Chaves QSettings — definidas aqui para evitar strings mágicas dispersas.
    constexpr const char* kKeyDataVersion          = "meta/dataVersion";
    constexpr const char* kKeyHighestLevel         = "progress/highestUnlockedLevel";
    constexpr const char* kKeyLastPlayedLevel      = "progress/lastPlayedLevel";
    constexpr const char* kKeySoundEnabled         = "settings/soundEnabled";
    constexpr const char* kKeyMusicEnabled         = "settings/musicEnabled";
    constexpr const char* kKeyLanguage             = "settings/language";
    constexpr const char* kKeyColorTheme           = "settings/colorTheme";
    constexpr const char* kKeyMasterVolume         = "settings/masterVolume";
    constexpr const char* kKeySessionLevelId       = "session/levelId";
    constexpr const char* kKeySessionMoveCount     = "session/moveCount";
    constexpr const char* kKeySessionElapsedSecs   = "session/elapsedSecs";
    constexpr const char* kKeySessionIsActive      = "session/isActive";
    constexpr const char* kKeyTotalPlaytimeSecs    = "stats/totalPlaytimeSecs";
    constexpr const char* kKeyTotalCompletions     = "stats/totalCompletions";
}

// ─────────────────────────────────────────────────────────────────────────────
// Classe PersistenceManager
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Gestor de persistência de dados do Bus Jam Puzzle.
 *
 * Instância única criada no main() e injetada no GameController.
 * Não herda de QObject — comunicação síncrona com o GameController.
 */
class PersistenceManager
{
public:
    // ─── Construção e destruição ───────────────────────────────────────────

    /**
     * @brief Constrói o manager e inicializa QSettings.
     *
     * @param organizationName Nome da organização (ex.: "MyStudio").
     * @param applicationName  Nome da aplicação (ex.: "BusJamPuzzle").
     *
     * Ao construir:
     *  1. Abre QSettings com o escopo UserScope.
     *  2. Carrega (ou inicializa) o ficheiro JSON de resultados.
     *  3. Executa migração de versão se necessário.
     */
    explicit PersistenceManager(const QString& organizationName,
                                const QString& applicationName);

    /**
     * @brief Destrutor. Chama flush() para garantir escrita imediata.
     */
    ~PersistenceManager();

    // Não copiável, não movível.
    PersistenceManager(const PersistenceManager&)            = delete;
    PersistenceManager& operator=(const PersistenceManager&) = delete;
    PersistenceManager(PersistenceManager&&)                 = delete;
    PersistenceManager& operator=(PersistenceManager&&)      = delete;

    // ─── Progresso ─────────────────────────────────────────────────────────

    /**
     * @brief Devolve o id do nível mais alto desbloqueado.
     *
     * @return Id do nível; 1 se nunca guardado.
     */
    [[nodiscard]] int loadHighestUnlockedLevel() const;

    /**
     * @brief Guarda o id do nível mais alto desbloqueado.
     *
     * Só guarda se newLevel > valor atual (nunca regride).
     *
     * @param newLevel Id do nível a guardar.
     * @return true se o valor foi actualizado; false se era igual ou inferior.
     */
    bool saveHighestUnlockedLevel(int newLevel);

    /**
     * @brief Devolve o id do último nível jogado.
     *
     * @return Id do nível; -1 se nunca guardado.
     */
    [[nodiscard]] int loadLastPlayedLevel() const;

    /**
     * @brief Guarda o id do último nível jogado.
     *
     * @param levelId Id do nível.
     */
    void saveLastPlayedLevel(int levelId);

    // ─── Resultados por nível (best scores) ────────────────────────────────

    /**
     * @brief Devolve o melhor resultado guardado para um nível.
     *
     * @param levelId Id do nível.
     * @return SavedLevelResult preenchido; isValid()==false se sem dados.
     */
    [[nodiscard]] SavedLevelResult loadLevelResult(int levelId) const;

    /**
     * @brief Actualiza o melhor resultado de um nível com novos dados.
     *
     * Só actualiza cada campo se o novo valor for melhor que o guardado:
     *  - score   : maior é melhor.
     *  - moves   : menor é melhor.
     *  - time    : menor é melhor.
     *  - stars   : maior é melhor.
     * Sempre incrementa completionCount e actualiza lastPlayed.
     *
     * @param levelId Id do nível.
     * @param score   Pontuação obtida.
     * @param moves   Jogadas usadas.
     * @param secs    Tempo em segundos.
     * @param stars   Estrelas obtidas (0–3).
     * @return true se pelo menos um campo foi actualizado (novo recorde).
     */
    bool saveLevelResult(int levelId, int score, int moves, int secs, int stars);

    /**
     * @brief Indica se um nível foi alguma vez completado.
     *
     * @param levelId Id do nível.
     * @return true se completionCount > 0.
     */
    [[nodiscard]] bool isLevelCompleted(int levelId) const;

    /**
     * @brief Devolve o número de estrelas obtidas num nível.
     *
     * @param levelId Id do nível.
     * @return 0–3; 0 se nunca completado.
     */
    [[nodiscard]] int starsForLevel(int levelId) const;

    /**
     * @brief Devolve os resultados de todos os níveis com dados guardados.
     *
     * @return Lista de SavedLevelResult ordenada por levelId crescente.
     */
    [[nodiscard]] QList<SavedLevelResult> loadAllLevelResults() const;

    // ─── Definições ────────────────────────────────────────────────────────

    /**
     * @brief Carrega todas as definições do jogador.
     *
     * @return GameSettings com os valores guardados ou valores por omissão.
     */
    [[nodiscard]] GameSettings loadSettings() const;

    /**
     * @brief Guarda todas as definições do jogador.
     *
     * @param settings Definições a guardar.
     */
    void saveSettings(const GameSettings& settings);

    /**
     * @brief Guarda apenas o estado do som (atalho para o toggle em jogo).
     */
    void saveSoundEnabled(bool enabled);

    /**
     * @brief Guarda apenas o estado da música.
     */
    void saveMusicEnabled(bool enabled);

    // ─── Estado de sessão ──────────────────────────────────────────────────

    /**
     * @brief Guarda o estado da sessão em curso.
     *
     * Chamado pelo GameController quando a aplicação vai para segundo plano
     * ou quando o utilizador pausa o jogo.
     *
     * @param state Estado a guardar.
     */
    void saveSessionState(const SessionState& state);

    /**
     * @brief Carrega o estado da sessão guardada.
     *
     * @return SessionState; isActive==false se não houver sessão guardada.
     */
    [[nodiscard]] SessionState loadSessionState() const;

    /**
     * @brief Limpa o estado de sessão guardado.
     *
     * Chamado quando o nível é concluído ou abandonado normalmente.
     */
    void clearSessionState();

    // ─── Estatísticas globais ──────────────────────────────────────────────

    /**
     * @brief Devolve o tempo total de jogo em segundos.
     */
    [[nodiscard]] qint64 loadTotalPlaytimeSecs() const;

    /**
     * @brief Acrescenta segundos ao tempo total de jogo.
     *
     * @param secs Segundos a acrescentar.
     */
    void addPlaytimeSecs(qint64 secs);

    /**
     * @brief Devolve o número total de níveis completados (todas as tentativas).
     */
    [[nodiscard]] int loadTotalCompletions() const;

    // ─── Reset ─────────────────────────────────────────────────────────────

    /**
     * @brief Apaga todo o progresso e resultados, mantendo as definições.
     *
     * Equivalente a "Recomeçar do zero" no menu de definições.
     * As definições de som, música e idioma são preservadas.
     */
    void resetProgress();

    /**
     * @brief Apaga absolutamente tudo (progresso + definições + sessão).
     *
     * Usado em testes unitários e no botão "Reset completo".
     */
    void resetAll();

    // ─── I/O e sincronização ──────────────────────────────────────────────

    /**
     * @brief Força escrita imediata de QSettings para disco.
     *
     * QSettings escreve de forma lazy por omissão. Chamar flush()
     * garante que os dados estão em disco antes de um evento crítico
     * (ex.: fecho da aplicação, bloqueio do sistema).
     */
    void flush();

    /**
     * @brief Devolve o caminho do ficheiro QSettings em uso.
     */
    [[nodiscard]] QString settingsFilePath() const;

    /**
     * @brief Devolve o caminho do ficheiro JSON de resultados.
     */
    [[nodiscard]] QString resultsFilePath() const;

    /**
     * @brief Indica se os dados em disco estão intactos e legíveis.
     *
     * Verifica a versão de esquema e a integridade do JSON de resultados.
     *
     * @return true se os dados são válidos.
     */
    [[nodiscard]] bool isDataIntact() const;

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual para logs e depuração.
     *
     * Formato:
     * "PersistenceManager[highestLevel=5 results=3 sessionActive=false]"
     */
    [[nodiscard]] QString toString() const;

private:
    // ─── Membros ───────────────────────────────────────────────────────────

    QSettings settings_;        ///< Armazenamento chave/valor (QSettings).
    QString   resultsFilePath_; ///< Caminho do ficheiro JSON de resultados.

    /// Cache em memória dos resultados por nível (evita reler JSON a cada acesso).
    mutable QHash<int, SavedLevelResult> resultsCache_;

    /// true se resultsCache_ está sincronizado com o disco.
    mutable bool cacheLoaded_ { false };

    // ─── Auxiliares privados ──────────────────────────────────────────────

    /**
     * @brief Determina o caminho do ficheiro de resultados no diretório
     *        de dados da aplicação.
     *
     * Usa QStandardPaths::AppDataLocation. Cria o diretório se não existir.
     *
     * @param appName Nome da aplicação (usado no path).
     * @return Caminho absoluto do ficheiro JSON.
     */
    [[nodiscard]] static QString resolveResultsFilePath(const QString& appName);

    /**
     * @brief Carrega o ficheiro JSON de resultados para resultsCache_.
     *
     * Chamado lazily na primeira leitura de resultados.
     * Se o ficheiro não existir, inicializa cache vazia.
     */
    void loadResultsFromDisk() const;

    /**
     * @brief Grava resultsCache_ para o ficheiro JSON de resultados.
     *
     * @return true se a escrita foi bem-sucedida.
     */
    bool saveResultsToDisk() const;

    /**
     * @brief Serializa um SavedLevelResult para QJsonObject.
     */
    [[nodiscard]] static QJsonObject resultToJson(const SavedLevelResult& r);

    /**
     * @brief Deserializa um QJsonObject para SavedLevelResult.
     *
     * @param obj    Objeto JSON de origem.
     * @param levelId Id do nível (chave no mapa externo).
     * @return SavedLevelResult; isValid()==false se obj for inválido.
     */
    [[nodiscard]] static SavedLevelResult resultFromJson(const QJsonObject& obj,
                                                         int levelId);

    /**
     * @brief Verifica a versão dos dados em disco e executa migração
     *        se a versão for inferior à atual.
     *
     * Chamado no construtor.
     */
    void migrateIfNeeded();

    /**
     * @brief Inicializa QSettings com os valores por omissão se estiver vazio.
     *
     * Chamado no construtor após migração.
     */
    void initializeDefaults();
};
