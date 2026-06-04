/**
 * @file Passenger.cpp
 * @brief Implementação da classe Passenger para o jogo Bus Jam Puzzle.
 *
 * Todas as transições de estado são validadas antes de qualquer mutação.
 * Violações de invariantes são reportadas via qWarning; nunca são lançadas
 * exceções em runtime (contrato de Qt).
 *
 * Máquina de estados:
 *
 *   Waiting ──iniciarEmbarque()──► Boarding ──concluirEmbarque()──► Boarded
 *      ▲                                                               │
 *      └───────────────────── reset() ◄───────────────────────────────┘
 */

#include "Passenger.h"

#include <QDebug>

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────

Passenger::Passenger(int      id,
                     BusColor color,
                     QPoint   cellPos)
    : id_          { id }
    , color_       { color }
    , startCellPos_{ cellPos }
    , cellPos_     { cellPos }
    , state_       { PassengerState::Waiting }
    , busId_       { PassengerConstants::kNoBusId }
{
    qDebug().noquote()
        << "Passenger criado:" << toString();
}

// ─────────────────────────────────────────────────────────────────────────────
// Identificação
// ─────────────────────────────────────────────────────────────────────────────

int Passenger::id() const noexcept
{
    return id_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Cor
// ─────────────────────────────────────────────────────────────────────────────

BusColor Passenger::color() const noexcept
{
    return color_;
}

QString Passenger::colorName() const
{
    // Delega na lógica de Bus.h — a tabela de nomes é a mesma.
    // Quando BusColor migrar para Enums.h, ambos os ficheiros importarão
    // uma função colorToString() partilhada. Por ora, repetimos aqui
    // para manter Passenger.cpp autossuficiente sem dependência de Bus.cpp.
    switch (color_)
    {
        case BusColor::Red:    return QStringLiteral("Red");
        case BusColor::Blue:   return QStringLiteral("Blue");
        case BusColor::Green:  return QStringLiteral("Green");
        case BusColor::Yellow: return QStringLiteral("Yellow");
        case BusColor::Purple: return QStringLiteral("Purple");
    }
    Q_UNREACHABLE();
    return QStringLiteral("Unknown");
}

// ─────────────────────────────────────────────────────────────────────────────
// Posição
// ─────────────────────────────────────────────────────────────────────────────

QPoint Passenger::cellPosition() const noexcept
{
    return cellPos_;
}

int Passenger::row() const noexcept
{
    // Convenção: QPoint(x=col, y=row), idêntica à usada em Bus.
    return cellPos_.y();
}

int Passenger::col() const noexcept
{
    return cellPos_.x();
}

// ─────────────────────────────────────────────────────────────────────────────
// Estado
// ─────────────────────────────────────────────────────────────────────────────

PassengerState Passenger::state() const noexcept
{
    return state_;
}

QString Passenger::stateName() const
{
    switch (state_)
    {
        case PassengerState::Waiting:  return QStringLiteral("Waiting");
        case PassengerState::Boarding: return QStringLiteral("Boarding");
        case PassengerState::Boarded:  return QStringLiteral("Boarded");
    }
    Q_UNREACHABLE();
    return QStringLiteral("Unknown");
}

bool Passenger::isWaiting() const noexcept
{
    return state_ == PassengerState::Waiting;
}

bool Passenger::isBoarding() const noexcept
{
    return state_ == PassengerState::Boarding;
}

bool Passenger::isBoarded() const noexcept
{
    return state_ == PassengerState::Boarded;
}

// ─────────────────────────────────────────────────────────────────────────────
// Associação com autocarro
// ─────────────────────────────────────────────────────────────────────────────

int Passenger::busId() const noexcept
{
    return busId_;
}

bool Passenger::hasAssignedBus() const noexcept
{
    return busId_ != PassengerConstants::kNoBusId;
}

// ─────────────────────────────────────────────────────────────────────────────
// Validação de compatibilidade
// ─────────────────────────────────────────────────────────────────────────────

bool Passenger::isCompatibleWith(BusColor busColor) const noexcept
{
    // Condição 1: o passageiro deve estar à espera.
    // Um passageiro em Boarding ou Boarded não pode ser atribuído novamente.
    if (!isWaiting())
        return false;

    // Condição 2: a cor deve coincidir exatamente.
    if (busColor != color_)
        return false;

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Ações de ciclo de vida
// ─────────────────────────────────────────────────────────────────────────────

bool Passenger::iniciarEmbarque(int assignedBusId, BusColor busColor)
{
    // Pré-condição 1: id de autocarro deve ser válido.
    if (assignedBusId == PassengerConstants::kNoBusId)
    {
        qWarning().noquote()
            << "Passenger::iniciarEmbarque — id=" << id_
            << ": assignedBusId inválido (kNoBusId). Operação recusada.";
        return false;
    }

    // Pré-condição 2: a cor do autocarro deve ser compatível.
    // Verifica separadamente de assertTransition para dar mensagem precisa.
    if (busColor != color_)
    {
        qWarning().noquote()
            << "Passenger::iniciarEmbarque — id=" << id_
            << ": cor incompatível. Passageiro=" << colorName()
            << "Autocarro=" << static_cast<int>(busColor)
            << ". Operação recusada.";
        return false;
    }

    // Pré-condição 3: o passageiro deve estar em Waiting.
    if (!assertTransition(PassengerState::Waiting,
                          PassengerState::Boarding,
                          "iniciarEmbarque"))
    {
        return false;
    }

    // Invariante: ao entrar em Boarding, busId_ deixa de ser kNoBusId.
    busId_  = assignedBusId;
    state_  = PassengerState::Boarding;

    qDebug().noquote()
        << "Passenger id=" << id_
        << "iniciou embarque no autocarro id=" << busId_;

    return true;
}

bool Passenger::concluirEmbarque()
{
    // Pré-condição: deve estar em Boarding.
    // Boarding garante que busId_ já foi definido por iniciarEmbarque().
    if (!assertTransition(PassengerState::Boarding,
                          PassengerState::Boarded,
                          "concluirEmbarque"))
    {
        return false;
    }

    // Neste ponto, busId_ != kNoBusId (invariante mantido por iniciarEmbarque).
    Q_ASSERT(busId_ != PassengerConstants::kNoBusId);

    state_ = PassengerState::Boarded;

    qDebug().noquote()
        << "Passenger id=" << id_
        << "embarcou com sucesso no autocarro id=" << busId_;

    return true;
}

void Passenger::reset() noexcept
{
    // Restaura todos os atributos mutáveis ao estado inicial do nível.
    // Os atributos const (id_, color_, startCellPos_) nunca são alterados.

    cellPos_ = startCellPos_;
    state_   = PassengerState::Waiting;
    busId_   = PassengerConstants::kNoBusId;

    qDebug().noquote()
        << "Passenger id=" << id_
        << "reiniciado para posição" << startCellPos_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString Passenger::toString() const
{
    return QString("Passenger[id=%1 color=%2 pos=(%3,%4) state=%5 busId=%6]")
        .arg(id_)
        .arg(colorName())
        .arg(col())
        .arg(row())
        .arg(stateName())
        .arg(busId_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

bool Passenger::assertTransition(PassengerState from,
                                  PassengerState to,
                                  const char*    caller) const
{
    if (state_ != from)
    {
        // Constrói nomes de estado para a mensagem sem depender de stateName()
        // (que seria uma chamada virtual se a classe fosse polimórfica).
        // Como é final, chamamos diretamente — mas isolamos aqui para clareza.
        auto stateLabel = [](PassengerState s) -> const char* {
            switch (s)
            {
                case PassengerState::Waiting:  return "Waiting";
                case PassengerState::Boarding: return "Boarding";
                case PassengerState::Boarded:  return "Boarded";
            }
            return "Unknown";
        };

        qWarning().noquote()
            << "Passenger::" << caller
            << " — id=" << id_
            << ": transição inválida."
            << "Estado atual=" << stateLabel(state_)
            << "Esperado=" << stateLabel(from)
            << "Destino pretendido=" << stateLabel(to)
            << ". Operação recusada.";

        return false;
    }

    return true;
}
