/**
 * @file Platform.cpp
 * @brief Implementação da classe Platform para o jogo Bus Jam Puzzle.
 *
 * Todas as transições de estado são validadas antes de qualquer mutação.
 * O invariante central é: busId_ != kNoBusId ↔ state_ == Occupied.
 * Violações de pré-condição são reportadas via qWarning sem lançar exceções.
 *
 * Máquina de estados:
 *
 *   Free ──receberBus()──► Occupied ──liberarPlatform()──► Free
 *    ▲                         │
 *    │                         └──concluirPlatform()──► Completed
 *    │                                                       │
 *    └──────────────── reset() ◄────────────────────────────┘
 */

#include "Platform.h"

#include <QDebug>

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────

Platform::Platform(int       id,
                   BusColor  color,
                   QPoint    position,
                   Direction exitDirection)
    : id_           { id }
    , color_        { color }
    , position_     { position }
    , exitDirection_{ exitDirection }
    , state_        { PlatformState::Free }
    , busId_        { PlatformConstants::kNoBusId }
{
    qDebug().noquote()
        << "Platform criada:" << toString();
}

// ─────────────────────────────────────────────────────────────────────────────
// Identificação
// ─────────────────────────────────────────────────────────────────────────────

int Platform::id() const noexcept
{
    return id_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Cor
// ─────────────────────────────────────────────────────────────────────────────

BusColor Platform::color() const noexcept
{
    return color_;
}

QString Platform::colorName() const
{
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

QPoint Platform::position() const noexcept
{
    return position_;
}

int Platform::row() const noexcept
{
    // Convenção: QPoint(x=col, y=row), consistente com Bus e Passenger.
    return position_.y();
}

int Platform::col() const noexcept
{
    return position_.x();
}

// ─────────────────────────────────────────────────────────────────────────────
// Direção de saída
// ─────────────────────────────────────────────────────────────────────────────

Direction Platform::exitDirection() const noexcept
{
    return exitDirection_;
}

QString Platform::exitDirectionName() const
{
    switch (exitDirection_)
    {
        case Direction::Up:    return QStringLiteral("Up");
        case Direction::Down:  return QStringLiteral("Down");
        case Direction::Left:  return QStringLiteral("Left");
        case Direction::Right: return QStringLiteral("Right");
    }
    Q_UNREACHABLE();
    return QStringLiteral("Unknown");
}

// ─────────────────────────────────────────────────────────────────────────────
// Estado
// ─────────────────────────────────────────────────────────────────────────────

PlatformState Platform::state() const noexcept
{
    return state_;
}

QString Platform::stateName() const
{
    return QString::fromLatin1(Platform::stateLabel(state_));
}

bool Platform::isFree() const noexcept
{
    return state_ == PlatformState::Free;
}

bool Platform::isOccupied() const noexcept
{
    return state_ == PlatformState::Occupied;
}

bool Platform::isCompleted() const noexcept
{
    return state_ == PlatformState::Completed;
}

// ─────────────────────────────────────────────────────────────────────────────
// Associação com autocarro
// ─────────────────────────────────────────────────────────────────────────────

int Platform::currentBusId() const noexcept
{
    return busId_;
}

bool Platform::hasBus() const noexcept
{
    return busId_ != PlatformConstants::kNoBusId;
}

// ─────────────────────────────────────────────────────────────────────────────
// Validação
// ─────────────────────────────────────────────────────────────────────────────

bool Platform::podeReceberBus(int busId, BusColor busColor) const noexcept
{
    // Condição 1: id de autocarro deve ser válido.
    if (busId < 0)
        return false;

    // Condição 2: a plataforma deve estar livre.
    // Uma plataforma Occupied ou Completed não aceita novos autocarros.
    if (!isFree())
        return false;

    // Condição 3: a cor do autocarro deve corresponder à cor da plataforma.
    if (busColor != color_)
        return false;

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Ações de ciclo de vida
// ─────────────────────────────────────────────────────────────────────────────

bool Platform::receberBus(int busId, BusColor busColor)
{
    // Valida o id antes de verificar o estado, para uma mensagem mais precisa.
    if (busId < 0)
    {
        qWarning().noquote()
            << "Platform::receberBus — id=" << id_
            << ": busId inválido (" << busId << "). Operação recusada.";
        return false;
    }

    // A cor é verificada antes da transição de estado para evitar
    // qualquer mutação em caso de incompatibilidade.
    if (busColor != color_)
    {
        qWarning().noquote()
            << "Platform::receberBus — id=" << id_
            << ": cor incompatível. Plataforma=" << colorName()
            << "Autocarro=" << busId
            << "corAutocarro=" << static_cast<int>(busColor)
            << ". Operação recusada.";
        return false;
    }

    // Valida a transição de estado: Free → Occupied.
    if (!assertTransition(PlatformState::Free,
                          PlatformState::Occupied,
                          "receberBus"))
    {
        return false;
    }

    // Todas as pré-condições satisfeitas: aplica a transição.
    busId_ = busId;
    state_ = PlatformState::Occupied;

    qDebug().noquote()
        << "Platform id=" << id_
        << "recebeu autocarro id=" << busId_
        << "(cor=" << colorName() << ")";

    return true;
}

bool Platform::liberarPlatform()
{
    // Valida a transição: Occupied → Free.
    if (!assertTransition(PlatformState::Occupied,
                          PlatformState::Free,
                          "liberarPlatform"))
    {
        return false;
    }

    const int busIdSaindo = busId_;

    // Limpa a associação antes de mudar o estado para manter
    // o invariante: state_ == Free → busId_ == kNoBusId.
    busId_ = PlatformConstants::kNoBusId;
    state_ = PlatformState::Free;

    qDebug().noquote()
        << "Platform id=" << id_
        << "libertada após saída do autocarro id=" << busIdSaindo
        << "(voltou a Free).";

    return true;
}

bool Platform::concluirPlatform()
{
    // Valida a transição: Occupied → Completed.
    if (!assertTransition(PlatformState::Occupied,
                          PlatformState::Completed,
                          "concluirPlatform"))
    {
        return false;
    }

    const int busIdSaindo = busId_;

    // O autocarro saiu: a associação é limpa.
    // O invariante Completed → busId_ == kNoBusId é estabelecido aqui.
    busId_ = PlatformConstants::kNoBusId;
    state_ = PlatformState::Completed;

    qDebug().noquote()
        << "Platform id=" << id_
        << "concluída após saída do autocarro id=" << busIdSaindo
        << "(estado final: Completed).";

    return true;
}

void Platform::reset() noexcept
{
    // Restaura todos os atributos mutáveis ao estado inicial do nível.
    // Os atributos const (id_, color_, position_, exitDirection_)
    // são, por definição, imutáveis.

    const PlatformState estadoAnterior = state_;
    const int           busIdAnterior  = busId_;

    state_ = PlatformState::Free;
    busId_ = PlatformConstants::kNoBusId;

    qDebug().noquote()
        << "Platform id=" << id_
        << "reiniciada de [state=" << Platform::stateLabel(estadoAnterior)
        << "busId=" << busIdAnterior << "] para Free.";
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString Platform::toString() const
{
    return QString("Platform[id=%1 color=%2 pos=(%3,%4) exit=%5 state=%6 busId=%7]")
        .arg(id_)
        .arg(colorName())
        .arg(col())
        .arg(row())
        .arg(exitDirectionName())
        .arg(stateName())
        .arg(busId_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

bool Platform::assertTransition(PlatformState from,
                                 PlatformState to,
                                 const char*   caller) const
{
    if (state_ != from)
    {
        qWarning().noquote()
            << "Platform::" << caller
            << " — id=" << id_
            << ": transição inválida."
            << "Estado atual=" << Platform::stateLabel(state_)
            << "Esperado=" << Platform::stateLabel(from)
            << "Destino pretendido=" << Platform::stateLabel(to)
            << ". Operação recusada.";
        return false;
    }

    return true;
}

const char* Platform::stateLabel(PlatformState s) noexcept
{
    switch (s)
    {
        case PlatformState::Free:      return "Free";
        case PlatformState::Occupied:  return "Occupied";
        case PlatformState::Completed: return "Completed";
    }
    Q_UNREACHABLE();
    return "Unknown";
}
