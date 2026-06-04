/**
 * @file Passenger.h
 * @brief Declaração da classe Passenger para o jogo Bus Jam Puzzle.
 *
 * A classe Passenger representa um passageiro individual no tabuleiro.
 * É uma entidade de domínio pura: sem dependência de QML ou qualquer
 * subsistema de apresentação.
 *
 * ### Ciclo de vida de um passageiro:
 *
 *  Waiting  ──► Boarding  ──► Boarded
 *    │                           │
 *    └─────── reset() ───────────┘
 *
 *  - Waiting  : está na sua célula, aguarda o autocarro correto.
 *  - Boarding : o autocarro chegou à plataforma; o embarque está em curso.
 *               Este estado é transitório (pode durar frames de animação).
 *  - Boarded  : entrou no autocarro. Estado terminal dentro do nível.
 *               Só reset() pode reverter.
 *
 * ### Invariantes da classe:
 *  - state_ == Boarding → busId_ != kNoBusId
 *  - state_ == Boarded  → busId_ != kNoBusId
 *  - state_ == Waiting  → busId_ == kNoBusId
 *  - Uma vez em Boarded, nenhuma transição é permitida (exceto reset).
 */

#pragma once

#include "Bus.h"   // BusColor — enum partilhado; Passenger não usa Bus diretamente.

#include <QString>
#include <QPoint>

// ─────────────────────────────────────────────────────────────────────────────
// Enum de estado do passageiro
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Estados possíveis de um passageiro durante um nível.
 *
 * A progressão é estritamente unidirecional: Waiting → Boarding → Boarded.
 * Nenhuma transição inversa é permitida excepto via reset().
 */
enum class PassengerState : quint8
{
    Waiting  = 0,  ///< Na célula do tabuleiro, aguarda o autocarro.
    Boarding = 1,  ///< Embarque em curso (autocarro na plataforma).
    Boarded  = 2   ///< Dentro do autocarro. Estado terminal no nível.
};

// ─────────────────────────────────────────────────────────────────────────────
// Constantes do módulo
// ─────────────────────────────────────────────────────────────────────────────

namespace PassengerConstants
{
    /// Sentinela: o passageiro ainda não está associado a nenhum autocarro.
    constexpr int kNoBusId = -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Classe Passenger
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Entidade de domínio que representa um passageiro no Bus Jam Puzzle.
 *
 * ### Relação com Bus:
 *  Passenger conhece apenas o *id* do autocarro que o transportou,
 *  nunca um ponteiro direto. Isso evita acoplamento bidirecional e
 *  simplifica o reset e a serialização.
 *
 * ### Thread-safety:
 *  A classe não é thread-safe. Todo o acesso deve ocorrer na thread
 *  do BoardEngine.
 */
class Passenger
{
public:
    // ─── Construção e destruição ───────────────────────────────────────────

    /**
     * @brief Constrói um passageiro com identidade e posição inicial.
     *
     * @param id       Identificador único no contexto do nível.
     * @param color    Cor do passageiro. Deve corresponder a uma cor de Bus.
     * @param cellPos  Posição da célula ocupada na grelha (col=x, row=y).
     */
    explicit Passenger(int      id,
                       BusColor color,
                       QPoint   cellPos);

    /** @brief Destrutor padrão. */
    ~Passenger() = default;

    // Passageiros são entidades únicas no tabuleiro — cópia desabilitada.
    Passenger(const Passenger&)            = delete;
    Passenger& operator=(const Passenger&) = delete;

    // Move permitido para armazenamento em containers Qt sem alocação extra.
    Passenger(Passenger&&)                 = default;
    Passenger& operator=(Passenger&&)      = default;

    // ─── Identificação ─────────────────────────────────────────────────────

    /**
     * @brief Devolve o identificador único do passageiro neste nível.
     */
    [[nodiscard]] int id() const noexcept;

    // ─── Cor ───────────────────────────────────────────────────────────────

    /**
     * @brief Devolve a cor do passageiro.
     *
     * Esta cor determina qual autocarro pode transportar este passageiro.
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
     * @brief Devolve a posição da célula que o passageiro ocupa no tabuleiro.
     *
     * Posição válida apenas enquanto state_ != Boarded.
     * Após embarque, a posição perde significado mas é mantida para reset().
     */
    [[nodiscard]] QPoint cellPosition() const noexcept;

    /**
     * @brief Devolve a linha da célula (atalho para cellPosition().y()).
     */
    [[nodiscard]] int row() const noexcept;

    /**
     * @brief Devolve a coluna da célula (atalho para cellPosition().x()).
     */
    [[nodiscard]] int col() const noexcept;

    // ─── Estado ────────────────────────────────────────────────────────────

    /**
     * @brief Devolve o estado atual do passageiro.
     */
    [[nodiscard]] PassengerState state() const noexcept;

    /**
     * @brief Representação legível do estado para logs e depuração.
     * @return "Waiting", "Boarding" ou "Boarded".
     */
    [[nodiscard]] QString stateName() const;

    /**
     * @brief Indica se o passageiro ainda está à espera no tabuleiro.
     */
    [[nodiscard]] bool isWaiting() const noexcept;

    /**
     * @brief Indica se o embarque está em curso (estado transitório).
     */
    [[nodiscard]] bool isBoarding() const noexcept;

    /**
     * @brief Indica se o passageiro já entrou no autocarro (estado terminal).
     */
    [[nodiscard]] bool isBoarded() const noexcept;

    // ─── Associação com autocarro ──────────────────────────────────────────

    /**
     * @brief Devolve o id do autocarro que transportou este passageiro.
     *
     * @return Id do autocarro, ou PassengerConstants::kNoBusId se ainda
     *         não estiver associado a nenhum autocarro.
     */
    [[nodiscard]] int busId() const noexcept;

    /**
     * @brief Indica se o passageiro já está associado a um autocarro.
     *
     * true quando state_ == Boarding ou state_ == Boarded.
     */
    [[nodiscard]] bool hasAssignedBus() const noexcept;

    // ─── Validação de compatibilidade ─────────────────────────────────────

    /**
     * @brief Verifica se este passageiro é compatível com um dado autocarro.
     *
     * Compatibilidade exige:
     *  - Mesma cor que o autocarro.
     *  - Passageiro ainda em estado Waiting.
     *
     * @param busColor Cor do autocarro candidato.
     * @return true se o embarque é permitido pelas regras de cor e estado.
     *
     * @note Esta verificação não consulta a capacidade do autocarro.
     *       Essa responsabilidade é do BoardEngine / Bus::podeEmbarcar().
     */
    [[nodiscard]] bool isCompatibleWith(BusColor busColor) const noexcept;

    // ─── Ações de ciclo de vida ────────────────────────────────────────────

    /**
     * @brief Inicia o processo de embarque, associando um autocarro.
     *
     * Transição: Waiting → Boarding.
     *
     * Pré-condições:
     *  - state_ == Waiting
     *  - assignedBusId ≠ PassengerConstants::kNoBusId
     *  - A cor do autocarro (passada como argumento) coincide com color_.
     *
     * @param assignedBusId  Id do autocarro que vai transportar este passageiro.
     * @param busColor       Cor do autocarro (usada para validar compatibilidade).
     * @return true se a transição foi executada; false se alguma pré-condição
     *         falhou (estado já avançado, id inválido, cor incompatível).
     */
    bool iniciarEmbarque(int assignedBusId, BusColor busColor);

    /**
     * @brief Conclui o embarque, marcando o passageiro como dentro do autocarro.
     *
     * Transição: Boarding → Boarded.
     *
     * Pré-condições:
     *  - state_ == Boarding
     *  - busId_ ≠ PassengerConstants::kNoBusId (garantido por iniciarEmbarque)
     *
     * @return true se a transição foi executada; false se state_ != Boarding.
     */
    bool concluirEmbarque();

    /**
     * @brief Restaura o passageiro ao estado inicial do nível.
     *
     * Utilizado pelo botão "Recomeçar". Restaura:
     *  - state_  → Waiting
     *  - busId_  → PassengerConstants::kNoBusId
     *
     * A posição e cor nunca mudam (são const).
     */
    void reset() noexcept;

    // ─── Utilitários ──────────────────────────────────────────────────────

    /**
     * @brief Representação textual do passageiro para logs e depuração.
     *
     * Formato:
     * "Passenger[id=3 color=Blue pos=(2,4) state=Waiting busId=-1]"
     */
    [[nodiscard]] QString toString() const;

private:
    // ─── Atributos imutáveis após construção ──────────────────────────────

    const int      id_;           ///< Identificador único no nível.
    const BusColor color_;        ///< Cor que determina autocarro compatível.
    const QPoint   startCellPos_; ///< Posição original na grelha (para reset).

    // ─── Estado mutável ───────────────────────────────────────────────────

    QPoint         cellPos_;      ///< Posição atual na grelha.
    PassengerState state_;        ///< Estado atual no ciclo Waiting→Boarding→Boarded.
    int            busId_;        ///< Id do autocarro associado; kNoBusId se nenhum.

    // ─── Auxiliares privados ──────────────────────────────────────────────

    /**
     * @brief Verifica e reporta se a transição de estado pedida é válida.
     *
     * @param from  Estado de origem esperado.
     * @param to    Estado de destino desejado.
     * @param caller Nome do método chamador (para mensagens de aviso).
     * @return true se state_ == from (transição possível).
     */
    [[nodiscard]] bool assertTransition(PassengerState from,
                                        PassengerState to,
                                        const char*    caller) const;
};
