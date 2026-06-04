/**
 * @file Platform.h
 * @brief Declaração da classe Platform para o jogo Bus Jam Puzzle.
 *
 * Uma plataforma é um ponto fixo na borda do tabuleiro onde um autocarro
 * de cor específica chega, embarca os passageiros compatíveis que ainda
 * aguardam, e de seguida sai definitivamente do tabuleiro.
 *
 * ### Ciclo de vida de uma plataforma num nível:
 *
 *  Free ──receberBus()──► Occupied ──liberarPlatform()──► Free
 *                                                          │
 *                              concluirPlatform() ─────────┘──► Completed
 *
 *  - Free      : disponível, aguarda o autocarro correto.
 *  - Occupied  : um autocarro está presente; embarque em curso.
 *  - Completed : o autocarro saiu com todos os passageiros possíveis.
 *                Estado terminal no nível (só reset() pode reverter).
 *
 * ### Nota sobre liberarPlatform() vs concluirPlatform():
 *  - liberarPlatform() → autocarro saiu mas a plataforma pode receber outro
 *    (útil se o design de nível futuro permitir múltiplos autocarros da mesma cor).
 *    Transição: Occupied → Free.
 *  - concluirPlatform() → autocarro saiu e a plataforma está encerrada para o nível.
 *    Transição: Occupied → Completed.
 *
 * ### Invariantes da classe:
 *  - state_ == Occupied  → busId_ != kNoBusId
 *  - state_ == Free      → busId_ == kNoBusId
 *  - state_ == Completed → busId_ == kNoBusId  (autocarro já saiu)
 *  - Uma vez em Completed, nenhum autocarro pode ser recebido (só reset).
 */

#pragma once

#include "Bus.h"   // BusColor, Direction

#include <QString>
#include <QPoint>

// ─────────────────────────────────────────────────────────────────────────────
// Enum de estado da plataforma
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Estados possíveis de uma plataforma durante um nível.
 *
 * A progressão nominal é: Free → Occupied → Completed.
 * A transição Occupied → Free é permitida para suportar situações em que
 * o autocarro parte antes de a plataforma ser marcada como concluída
 * (ex.: animação de saída ainda em curso quando o estado de jogo avança).
 */
enum class PlatformState : quint8
{
    Free      = 0,  ///< Disponível; nenhum autocarro presente.
    Occupied  = 1,  ///< Autocarro presente; embarque em curso.
    Completed = 2   ///< Autocarro saiu; plataforma encerrada no nível.
};

// ─────────────────────────────────────────────────────────────────────────────
// Constantes do módulo
// ─────────────────────────────────────────────────────────────────────────────

namespace PlatformConstants
{
    /// Sentinela: nenhum autocarro está atualmente associado à plataforma.
    constexpr int kNoBusId = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Classe Platform
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Entidade de domínio que representa uma plataforma de embarque.
 *
 * ### Relação com Bus:
 *  Platform conhece apenas o *id* do autocarro que a ocupa, nunca um
 *  ponteiro direto. O BoardEngine é responsável por coordenar as chamadas
 *  entre Bus e Platform.
 *
 * ### Relação com Passenger:
 *  Platform não gere passageiros diretamente. O BoardEngine determina
 *  quais os passageiros compatíveis e invoca Bus::embarcarPassageiro().
 *  Platform limita-se a validar e registar a presença do autocarro.
 *
 * ### Thread-safety:
 *  Não é thread-safe. Todo o acesso deve ocorrer na thread do BoardEngine.
 */
class Platform
{
public:
    // ─── Construção e destruição ───────────────────────────────────────────

    /**
     * @brief Constrói uma plataforma com todos os atributos fixos do nível.
     *
     * @param id           Identificador único no contexto do nível.
     * @param color        Cor esperada: só autocarros desta cor são aceites.
     * @param position     Posição fixa na grelha (col=x, row=y).
     * @param exitDirection Direção pela qual o autocarro entra e sai.
     */
    explicit Platform(int       id,
                      BusColor  color,
                      QPoint    position,
                      Direction exitDirection);

    /** @brief Destrutor padrão. */
    ~Platform() = default;

    // Plataformas são entidades fixas e únicas no tabuleiro — cópia desabilitada.
    Platform(const Platform&)            = delete;
    Platform& operator=(const Platform&) = delete;

    // Move permitido para armazenamento em containers Qt.
    Platform(Platform&&)                 = default;
    Platform& operator=(Platform&&)      = default;

    // ─── Identificação ─────────────────────────────────────────────────────

    /**
     * @brief Devolve o identificador único da plataforma neste nível.
     */
    [[nodiscard]] int id() const noexcept;

    // ─── Cor ───────────────────────────────────────────────────────────────

    /**
     * @brief Devolve a cor esperada da plataforma.
     *
     * Apenas autocarros com esta cor podem ser recebidos.
     * Nunca muda após a construção.
     */
    [[nodiscard]] BusColor color() const noexcept;

    /**
     * @brief Representação legível da cor para logs e depuração.
     * @return Ex.: "Red", "Blue", "Green", "Yellow", "Purple".
     */
    [[nodiscard]] QString colorName() const;

    // ─── Posição ───────────────────────────────────────────────────────────

    /**
     * @brief Devolve a posição fixa da plataforma na grelha.
     *
     * A posição nunca muda: plataformas são estáticas no tabuleiro.
     */
    [[nodiscard]] QPoint position() const noexcept;

    /**
     * @brief Devolve a linha da plataforma (atalho para position().y()).
     */
    [[nodiscard]] int row() const noexcept;

    /**
     * @brief Devolve a coluna da plataforma (atalho para position().x()).
     */
    [[nodiscard]] int col() const noexcept;

    // ─── Direção de saída ─────────────────────────────────────────────────

    /**
     * @brief Devolve a direção pela qual o autocarro entra e sai.
     *
     * Define o alinhamento visual da plataforma no tabuleiro e
     * é usada pelo MovementSolver para calcular o trajecto do autocarro.
     * Nunca muda após a construção.
     */
    [[nodiscard]] Direction exitDirection() const noexcept;

    /**
     * @brief Representação legível da direção para logs e depuração.
     * @return "Up", "Down", "Left" ou "Right".
     */
    [[nodiscard]] QString exitDirectionName() const;

    // ─── Estado ────────────────────────────────────────────────────────────

    /**
     * @brief Devolve o estado atual da plataforma.
     */
    [[nodiscard]] PlatformState state() const noexcept;

    /**
     * @brief Representação legível do estado para logs e depuração.
     * @return "Free", "Occupied" ou "Completed".
     */
    [[nodiscard]] QString stateName() const;

    /**
     * @brief Indica se a plataforma está livre para receber um autocarro.
     */
    [[nodiscard]] bool isFree() const noexcept;

    /**
     * @brief Indica se a plataforma está atualmente ocupada por um autocarro.
     */
    [[nodiscard]] bool isOccupied() const noexcept;

    /**
     * @brief Indica se a plataforma concluiu o serviço no nível atual.
     *
     * Uma plataforma completed não aceita mais autocarros até ao reset().
     */
    [[nodiscard]] bool isCompleted() const noexcept;

    // ─── Associação com autocarro ──────────────────────────────────────────

    /**
     * @brief Devolve o id do autocarro que ocupa atualmente a plataforma.
     *
     * @return Id do autocarro, ou PlatformConstants::kNoBusId se não houver
     *         nenhum autocarro presente.
     */
    [[nodiscard]] int currentBusId() const noexcept;

    /**
     * @brief Indica se há um autocarro atualmente na plataforma.
     */
    [[nodiscard]] bool hasBus() const noexcept;

    // ─── Validação ─────────────────────────────────────────────────────────

    /**
     * @brief Verifica se a plataforma pode receber um determinado autocarro.
     *
     * Condições para receção:
     *  - A plataforma está em estado Free.
     *  - A cor do autocarro coincide com a cor esperada da plataforma.
     *  - O id do autocarro é válido (≥ 0).
     *
     * @param busId    Id do autocarro candidato.
     * @param busColor Cor do autocarro candidato.
     * @return true se a plataforma pode receber o autocarro.
     *
     * @note Este método é puramente de consulta: não altera o estado.
     *       Use receberBus() para efetuar a receção.
     */
    [[nodiscard]] bool podeReceberBus(int busId, BusColor busColor) const noexcept;

    // ─── Ações de ciclo de vida ────────────────────────────────────────────

    /**
     * @brief Regista a chegada de um autocarro à plataforma.
     *
     * Transição: Free → Occupied.
     *
     * Pré-condições validadas internamente:
     *  - state_ == Free
     *  - busColor == color_
     *  - busId ≥ 0
     *
     * @param busId    Id do autocarro que chegou.
     * @param busColor Cor do autocarro (validada contra color_).
     * @return true se a receção foi bem-sucedida; false se alguma
     *         pré-condição falhou.
     */
    bool receberBus(int busId, BusColor busColor);

    /**
     * @brief Liberta a plataforma sem a marcar como concluída.
     *
     * Transição: Occupied → Free.
     *
     * Use este método quando o autocarro sai mas a plataforma pode ainda
     * receber outro autocarro da mesma cor no mesmo nível.
     *
     * Pré-condição: state_ == Occupied.
     *
     * @return true se a libertação foi bem-sucedida; false se state_ != Occupied.
     */
    bool liberarPlatform();

    /**
     * @brief Liberta a plataforma e marca-a como concluída para o nível.
     *
     * Transição: Occupied → Completed.
     *
     * Use este método quando o autocarro saiu e a plataforma está
     * encerrada para o resto do nível.
     *
     * Pré-condição: state_ == Occupied.
     *
     * @return true se a conclusão foi bem-sucedida; false se state_ != Occupied.
     */
    bool concluirPlatform();

    /**
     * @brief Restaura a plataforma ao estado inicial do nível.
     *
     * Utilizado pelo botão "Recomeçar". Restaura:
     *  - state_  → Free
     *  - busId_  → PlatformConstants::kNoBusId
     *
     * A posição, cor e direção nunca mudam (são const).
     */
    void reset() noexcept;

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual da plataforma para logs e depuração.
     *
     * Formato:
     * "Platform[id=2 color=Blue pos=(0,3) exit=Right state=Free busId=-1]"
     */
    [[nodiscard]] QString toString() const;

private:
    // ─── Atributos imutáveis após construção ──────────────────────────────

    const int       id_;            ///< Identificador único no nível.
    const BusColor  color_;         ///< Cor esperada do autocarro.
    const QPoint    position_;      ///< Posição fixa na grelha.
    const Direction exitDirection_; ///< Direção de entrada/saída do autocarro.

    // ─── Estado mutável ───────────────────────────────────────────────────

    PlatformState state_;   ///< Estado atual no ciclo Free→Occupied→Completed.
    int           busId_;   ///< Id do autocarro presente; kNoBusId se nenhum.

    // ─── Auxiliares privados ──────────────────────────────────────────────

    /**
     * @brief Valida e reporta uma transição de estado.
     *
     * @param from   Estado de origem esperado.
     * @param to     Estado de destino desejado.
     * @param caller Nome do método chamador (para mensagens de aviso).
     * @return true se state_ == from (transição possível).
     */
    [[nodiscard]] bool assertTransition(PlatformState from,
                                        PlatformState to,
                                        const char*   caller) const;

    /**
     * @brief Devolve o nome textual de um estado de plataforma.
     *
     * Usado internamente por assertTransition e stateName.
     * Separado de stateName() para poder ser chamado com estados arbitrários.
     */
    [[nodiscard]] static const char* stateLabel(PlatformState s) noexcept;
};
