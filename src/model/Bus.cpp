/**
 * @file Bus.cpp
 * @brief Implementação da classe Bus para o jogo Bus Jam Puzzle.
 *
 * Todos os invariantes descritos no header são mantidos aqui.
 * Erros de programação (pré-condições violadas) são reportados via
 * qWarning / Q_ASSERT para manter o contrato de não-exceção do Qt.
 * Erros de configuração (capacidade inválida) lançam std::invalid_argument
 * no construtor, antes de qualquer estado ser criado.
 */

#include "Bus.h"

#include <QDebug>
#include <stdexcept>
#include <algorithm>  // std::find

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────

Bus::Bus(int       id,
         BusColor  color,
         int       capacity,
         QPoint    startPos,
         Direction direction)
    : id_           { id }
    , color_        { color }
    , capacity_     { capacity }      // validado imediatamente abaixo
    , startPosition_{ startPos }
    , direction_    { direction }
    , position_     { startPos }
    , boardedCount_ { 0 }
    , isActive_     { true }
    , hasLeft_      { false }
    , isAtPlatform_ { false }
{
    // Validação de capacidade — único ponto onde lançamos exceção,
    // porque ocorre antes que qualquer invariante seja estabelecido.
    if (!Bus::isValidCapacity(capacity_))
    {
        throw std::invalid_argument(
            QString("Bus id=%1: capacidade inválida (%2). "
                    "Valores permitidos: 4, 6, 8, 12.")
                .arg(id_)
                .arg(capacity_)
                .toStdString()
        );
    }

    qDebug().noquote()
        << "Bus criado:" << toString();
}

// ─────────────────────────────────────────────────────────────────────────────
// Identificação
// ─────────────────────────────────────────────────────────────────────────────

int Bus::id() const noexcept
{
    return id_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Cor
// ─────────────────────────────────────────────────────────────────────────────

BusColor Bus::color() const noexcept
{
    return color_;
}

QString Bus::colorName() const
{
    switch (color_)
    {
        case BusColor::Red:    return QStringLiteral("Red");
        case BusColor::Blue:   return QStringLiteral("Blue");
        case BusColor::Green:  return QStringLiteral("Green");
        case BusColor::Yellow: return QStringLiteral("Yellow");
        case BusColor::Purple: return QStringLiteral("Purple");
    }
    // Nunca deve chegar aqui com um enum válido.
    Q_UNREACHABLE();
    return QStringLiteral("Unknown");
}

// ─────────────────────────────────────────────────────────────────────────────
// Capacidade e passageiros
// ─────────────────────────────────────────────────────────────────────────────

int Bus::capacity() const noexcept
{
    return capacity_;
}

int Bus::boardedCount() const noexcept
{
    return boardedCount_;
}

bool Bus::isFull() const noexcept
{
    return boardedCount_ >= capacity_;
}

int Bus::availableSeats() const noexcept
{
    return capacity_ - boardedCount_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Posição
// ─────────────────────────────────────────────────────────────────────────────

QPoint Bus::position() const noexcept
{
    return position_;
}

int Bus::row() const noexcept
{
    // QPoint usa (x=col, y=row) por convenção neste projeto.
    return position_.y();
}

int Bus::col() const noexcept
{
    return position_.x();
}

void Bus::setPosition(QPoint newPos)
{
    if (hasLeft_)
    {
        qWarning().noquote()
            << "Bus::setPosition — autocarro id=" << id_
            << "já saiu do tabuleiro. Operação ignorada.";
        return;
    }

    position_ = newPos;
}

// ─────────────────────────────────────────────────────────────────────────────
// Direção
// ─────────────────────────────────────────────────────────────────────────────

Direction Bus::direction() const noexcept
{
    return direction_;
}

QString Bus::directionName() const
{
    switch (direction_)
    {
        case Direction::Up:    return QStringLiteral("Up");
        case Direction::Down:  return QStringLiteral("Down");
        case Direction::Left:  return QStringLiteral("Left");
        case Direction::Right: return QStringLiteral("Right");
    }
    Q_UNREACHABLE();
    return QStringLiteral("Unknown");
}

QPoint Bus::nextPosition() const noexcept
{
    return position_ + Bus::directionDelta(direction_);
}

// ─────────────────────────────────────────────────────────────────────────────
// Estado
// ─────────────────────────────────────────────────────────────────────────────

bool Bus::isActive() const noexcept
{
    return isActive_;
}

bool Bus::hasLeft() const noexcept
{
    return hasLeft_;
}

bool Bus::isAtPlatform() const noexcept
{
    return isAtPlatform_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Ações principais
// ─────────────────────────────────────────────────────────────────────────────

bool Bus::mover()
{
    // Pré-condição 1: autocarro deve estar ativo.
    if (!isActive_)
    {
        qWarning().noquote()
            << "Bus::mover — autocarro id=" << id_
            << "está inativo. Movimento recusado.";
        return false;
    }

    // Pré-condição 2: autocarro não pode ter saído do tabuleiro.
    if (hasLeft_)
    {
        qWarning().noquote()
            << "Bus::mover — autocarro id=" << id_
            << "já saiu do tabuleiro. Movimento recusado.";
        return false;
    }

    // Um autocarro numa plataforma aguarda embarque; não se move por aqui.
    // O BoardEngine é responsável por chamar sairDoTabuleiro() quando
    // o embarque estiver concluído.
    if (isAtPlatform_)
    {
        qWarning().noquote()
            << "Bus::mover — autocarro id=" << id_
            << "está numa plataforma. Use sairDoTabuleiro() após embarque.";
        return false;
    }

    // Executa o deslocamento de uma célula na direção fixa.
    // Nota: a validação de limites e colisões é responsabilidade
    // do BoardEngine / MovementSolver, que chama este método
    // apenas quando o caminho está livre.
    const QPoint anterior = position_;
    position_ = nextPosition();

    qDebug().noquote()
        << "Bus id=" << id_
        << "moveu" << directionName()
        << "de" << anterior
        << "para" << position_;

    return true;
}

bool Bus::podeEmbarcar(BusColor passengerColor) const noexcept
{
    // Regra 1: autocarro deve estar ativo e não ter saído.
    if (!isActive_ || hasLeft_)
        return false;

    // Regra 2: autocarro deve estar numa plataforma de embarque.
    if (!isAtPlatform_)
        return false;

    // Regra 3: autocarro não pode estar cheio.
    if (isFull())
        return false;

    // Regra 4: a cor do passageiro deve coincidir com a do autocarro.
    if (passengerColor != color_)
        return false;

    return true;
}

bool Bus::embarcarPassageiro(BusColor passengerColor)
{
    // Verifica todas as condições de negócio antes de alterar estado.
    if (!podeEmbarcar(passengerColor))
    {
        qWarning().noquote()
            << "Bus::embarcarPassageiro — autocarro id=" << id_
            << "recusou passageiro de cor"
            << static_cast<int>(passengerColor)
            << ". Condições: ativo=" << isActive_
            << "naPlataforma=" << isAtPlatform_
            << "cheio=" << isFull()
            << "corCompativel=" << (passengerColor == color_);
        return false;
    }

    ++boardedCount_;

    qDebug().noquote()
        << "Bus id=" << id_
        << ": passageiro embarcou. Ocupação:"
        << boardedCount_ << "/" << capacity_;

    return true;
}

bool Bus::sairDoTabuleiro()
{
    if (hasLeft_)
    {
        qWarning().noquote()
            << "Bus::sairDoTabuleiro — autocarro id=" << id_
            << "já tinha saído anteriormente. Operação ignorada.";
        return false;
    }

    // Transição de estado: ativo → saído.
    // A posição fica inválida porque o autocarro não ocupa mais
    // nenhuma célula do tabuleiro.
    isActive_     = false;
    isAtPlatform_ = false;
    hasLeft_      = true;
    position_     = BusConstants::InvalidPosition;

    qDebug().noquote()
        << "Bus id=" << id_
        << "saiu do tabuleiro com"
        << boardedCount_ << "passageiros.";

    return true;
}

void Bus::setAtPlatform(bool atPlatform) noexcept
{
    if (hasLeft_)
    {
        // Autocarro já saiu — ignorar silenciosamente.
        // Pode ocorrer em condições de corrida no engine; não é erro fatal.
        return;
    }

    if (!isActive_)
    {
        qWarning().noquote()
            << "Bus::setAtPlatform — autocarro id=" << id_
            << "está inativo. Estado não alterado.";
        return;
    }

    isAtPlatform_ = atPlatform;

    qDebug().noquote()
        << "Bus id=" << id_
        << (atPlatform ? "chegou à plataforma." : "saiu da plataforma.");
}

void Bus::reset() noexcept
{
    // Restaura todos os atributos mutáveis ao estado inicial do nível.
    // Os atributos const (id_, color_, capacity_, startPosition_, direction_)
    // são, por definição, imutáveis e não precisam de reset.

    position_     = startPosition_;
    boardedCount_ = 0;
    isActive_     = true;
    hasLeft_      = false;
    isAtPlatform_ = false;

    qDebug().noquote()
        << "Bus id=" << id_ << "reiniciado para posição" << startPosition_;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilitários
// ─────────────────────────────────────────────────────────────────────────────

QString Bus::toString() const
{
    return QString("Bus[id=%1 color=%2 cap=%3 boarded=%4 "
                   "pos=(%5,%6) dir=%7 "
                   "active=%8 atPlatform=%9 hasLeft=%10]")
        .arg(id_)
        .arg(colorName())
        .arg(capacity_)
        .arg(boardedCount_)
        .arg(col())
        .arg(row())
        .arg(directionName())
        .arg(isActive_     ? "true" : "false")
        .arg(isAtPlatform_ ? "true" : "false")
        .arg(hasLeft_      ? "true" : "false");
}

bool Bus::isValidCapacity(int capacity) noexcept
{
    // Verifica explicitamente cada valor válido.
    // Preferido a um range (ex.: capacity % 2 == 0 && capacity >= 4)
    // porque a regra de negócio admite apenas estes quatro valores exatos.
    return capacity == 4
        || capacity == 6
        || capacity == 8
        || capacity == 12;
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

QPoint Bus::directionDelta(Direction dir) noexcept
{
    // Convenção de coordenadas da grelha:
    //   x = coluna (aumenta para a direita)
    //   y = linha  (aumenta para baixo)
    //
    // QPoint(deltaCol, deltaRow)
    switch (dir)
    {
        case Direction::Up:    return QPoint( 0, -1);
        case Direction::Down:  return QPoint( 0,  1);
        case Direction::Left:  return QPoint(-1,  0);
        case Direction::Right: return QPoint( 1,  0);
    }
    Q_UNREACHABLE();
    return QPoint(0, 0);
}
