/**
 * @file LevelManager.h
 * @brief Declaração da classe LevelManager para o jogo Bus Jam Puzzle.
 *
 * LevelManager gere a coleção completa de níveis disponíveis no jogo:
 * descoberta, carregamento, progressão e cache de metadados.
 *
 * ### Responsabilidades:
 *  - Descobrir ficheiros de nível num diretório ou nos recursos Qt.
 *  - Carregar metadados de todos os níveis ao iniciar (operação leve).
 *  - Fornecer Boards completos ao GameController quando um nível é selecionado.
 *  - Gerir a cache de Boards (evitar recarregamentos desnecessários).
 *  - Determinar o próximo nível na sequência.
 *  - Validar níveis disponíveis e reportar problemas de configuração.
 *
 * ### O que LevelManager NÃO faz:
 *  - Não persiste progresso do jogador (PersistenceManager).
 *  - Não controla qual nível está desbloqueado (GameController).
 *  - Não executa lógica de jogo (BoardEngine).
 *
 * ### Descoberta de níveis:
 *  O LevelManager suporta dois modos de descoberta:
 *
 *  1. **Diretório do sistema de ficheiros** (desenvolvimento / conteúdo externo):
 *     Todos os ficheiros `*.json` num diretório são carregados por ordem
 *     alfabética. O id do nível é lido do JSON, não inferido do nome do ficheiro.
 *
 *  2. **Recursos Qt** (produção / app store):
 *     Ficheiros listados explicitamente em `levels.qrc` ou descobertos via
 *     QDirIterator no prefixo ":/levels/". Compilados no binário.
 *
 * ### Cache de Boards:
 *  Por omissão, o LevelManager não guarda Boards em cache — cada chamada
 *  a buildBoardForLevel() lê e deserializa o JSON de novo. Para activar
 *  cache (útil em dispositivos lentos), chama setCachingEnabled(true).
 *  Com cache ativa, o Board é construído uma vez e reutilizado nos
 *  reinicios — o BoardEngine chama Board::reset() em vez de reconstruir.
 *
 * ### Ordenação de níveis:
 *  Os níveis são sempre apresentados ordenados por id crescente,
 *  independentemente da ordem de descoberta.
 */

#pragma once

#include "LevelLoader.h"   // LevelMetadata, LoadResult

#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <memory>

// ─────────────────────────────────────────────────────────────────────────────
// Constantes
// ─────────────────────────────────────────────────────────────────────────────

namespace LevelManagerConstants
{
    /// Prefixo de recursos Qt para ficheiros de nível.
    constexpr const char* kQrcLevelsPrefix = ":/levels/";

    /// Extensão esperada dos ficheiros de nível.
    constexpr const char* kLevelFileExtension = "*.json";

    /// Id sentinela: nenhum nível seguinte disponível.
    constexpr int kNoNextLevel = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Resultado de operações de gestão
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Resultado de uma operação do LevelManager que pode falhar.
 */
struct ManagerResult
{
    bool    success { false };
    QString errorMessage;

    [[nodiscard]] static ManagerResult ok()
    {
        ManagerResult r; r.success = true; return r;
    }

    [[nodiscard]] static ManagerResult fail(const QString& msg)
    {
        ManagerResult r; r.success = false; r.errorMessage = msg; return r;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Classe LevelManager
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Gestor da coleção de níveis do Bus Jam Puzzle.
 *
 * Instância única gerida pelo GameController. Não herda de QObject —
 * não emite sinais. Comunicação com o GameController é síncrona.
 */
class LevelManager
{
public:
    // ─── Construção e destruição ───────────────────────────────────────────

    /**
     * @brief Constrói o LevelManager sem carregar níveis.
     *
     * Chame loadFromDirectory() ou loadFromResources() para populá-lo.
     */
    explicit LevelManager();

    /** @brief Destrutor padrão. */
    ~LevelManager() = default;

    // Não copiável, não movível (gerido por ponteiro único no GameController).
    LevelManager(const LevelManager&)            = delete;
    LevelManager& operator=(const LevelManager&) = delete;
    LevelManager(LevelManager&&)                 = delete;
    LevelManager& operator=(LevelManager&&)      = delete;

    // ─── Carregamento de metadados ─────────────────────────────────────────

    /**
     * @brief Descobre e carrega metadados de todos os níveis num diretório.
     *
     * Pesquisa recursivamente ficheiros `*.json`. Ficheiros cujos metadados
     * falham a validação são ignorados (regista qWarning) — o manager
     * continua com os restantes.
     *
     * @param dirPath Caminho para o diretório de níveis.
     * @return ManagerResult com sucesso se pelo menos um nível foi carregado.
     */
    ManagerResult loadFromDirectory(const QString& dirPath);

    /**
     * @brief Carrega metadados de níveis a partir dos recursos Qt.
     *
     * Itera sobre ":/levels/*.json". Requer que os ficheiros estejam
     * listados no .qrc do projeto.
     *
     * @return ManagerResult com sucesso se pelo menos um nível foi carregado.
     */
    ManagerResult loadFromResources();

    /**
     * @brief Adiciona um único ficheiro de nível ao manager.
     *
     * Útil para testes unitários e para adicionar níveis DLC individualmente.
     *
     * @param filePath Caminho para o ficheiro JSON.
     * @return LoadResult com detalhes de sucesso ou falha.
     */
    LoadResult addLevel(const QString& filePath);

    // ─── Consultas de catálogo ─────────────────────────────────────────────

    /**
     * @brief Devolve o número de níveis disponíveis.
     */
    [[nodiscard]] int levelCount() const noexcept;

    /**
     * @brief Indica se o manager tem pelo menos um nível disponível.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Indica se um nível com o id dado existe no catálogo.
     *
     * @param levelId Id do nível a verificar.
     */
    [[nodiscard]] bool hasLevel(int levelId) const noexcept;

    /**
     * @brief Devolve os metadados de um nível específico.
     *
     * @param levelId Id do nível.
     * @return Ponteiro const para LevelMetadata, ou nullptr se não existir.
     */
    [[nodiscard]] const LevelMetadata* metadataForLevel(int levelId) const;

    /**
     * @brief Devolve a lista de metadados de todos os níveis, ordenada por id.
     */
    [[nodiscard]] QList<LevelMetadata> allLevelMetadata() const;

    /**
     * @brief Devolve a lista de ids de todos os níveis, ordenada.
     */
    [[nodiscard]] QList<int> allLevelIds() const;

    /**
     * @brief Devolve o id do primeiro nível disponível.
     *
     * @return Id do primeiro nível, ou -1 se o manager estiver vazio.
     */
    [[nodiscard]] int firstLevelId() const noexcept;

    /**
     * @brief Devolve o id do nível seguinte ao dado.
     *
     * @param currentLevelId Id do nível atual.
     * @return Id do próximo nível, ou LevelManagerConstants::kNoNextLevel
     *         se não houver mais níveis.
     */
    [[nodiscard]] int nextLevelId(int currentLevelId) const noexcept;

    /**
     * @brief Indica se existe um nível após o dado.
     */
    [[nodiscard]] bool hasNextLevel(int currentLevelId) const noexcept;

    /**
     * @brief Devolve o limite de tempo em segundos para um nível.
     *
     * @param levelId Id do nível.
     * @return Segundos; 0 se sem limite ou se o nível não existir.
     */
    [[nodiscard]] int timeLimitForLevel(int levelId) const noexcept;

    /**
     * @brief Devolve a dificuldade de um nível.
     *
     * @param levelId Id do nível.
     * @return Dificuldade 1–5; -1 se o nível não existir.
     */
    [[nodiscard]] int difficultyForLevel(int levelId) const noexcept;

    // ─── Construção de Board ───────────────────────────────────────────────

    /**
     * @brief Constrói um Board completo para o nível com o id dado.
     *
     * Se a cache estiver ativa e o Board já tiver sido construído,
     * devolve uma cópia do Board cacheado (via reset()).
     * Caso contrário, lê e deserializa o ficheiro JSON.
     *
     * @param levelId   Id do nível a construir.
     * @param outResult [out] Resultado detalhado do carregamento.
     * @return unique_ptr para o Board construído e populado;
     *         nullptr se o carregamento falhar.
     */
    [[nodiscard]] std::unique_ptr<Board> buildBoardForLevel(int         levelId,
                                                            LoadResult& outResult);

    // ─── Configuração ─────────────────────────────────────────────────────

    /**
     * @brief Activa ou desactiva a cache de Boards.
     *
     * Com cache ativa, o primeiro carregamento de cada nível é armazenado.
     * Chamadas subsequentes a buildBoardForLevel() para o mesmo id
     * constroem um novo Board a partir do mesmo ficheiro (o cache guarda
     * apenas o ficheiro path e os metadados, não o Board em si — o Board
     * é sempre reconstruído para garantir estado limpo).
     *
     * @note A cache de "Boards" nesta implementação é uma cache de
     *       metadados + file path, não de objetos Board. Boards são sempre
     *       construídos de raiz para garantir estado inicial limpo.
     *
     * @param enabled true para activar cache de metadados.
     */
    void setCachingEnabled(bool enabled) noexcept;

    /**
     * @brief Limpa todos os metadados e paths cacheados.
     */
    void clearCache() noexcept;

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual para logs e depuração.
     *
     * Formato: "LevelManager[levels=10 cached=true firstId=1 lastId=10]"
     */
    [[nodiscard]] QString toString() const;

private:
    // ─── Dados internos ───────────────────────────────────────────────────

    /// Metadados indexados por id de nível.
    QHash<int, LevelMetadata> metadataMap_;

    /// Ids ordenados por id crescente (cache de ordem).
    QList<int> sortedIds_;

    /// Cache ativa?
    bool cachingEnabled_ { false };

    // ─── Auxiliares privados ──────────────────────────────────────────────

    /**
     * @brief Insere ou actualiza metadados no mapa, mantendo sortedIds_ actualizado.
     *
     * @param meta Metadados a inserir.
     */
    void insertMetadata(const LevelMetadata& meta);

    /**
     * @brief Reconstrói sortedIds_ a partir das chaves de metadataMap_.
     *
     * Chamado após carregamento em massa para garantir ordenação.
     */
    void rebuildSortedIds();
};
