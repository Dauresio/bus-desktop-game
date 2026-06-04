/**
 * @file BusListModel.cpp
 * @brief Implementação de BusListModel.
 */

#include "BusListModel.h"
#include <QDebug>

// ─────────────────────────────────────────────────────────────────────────────
// Construtor
// ─────────────────────────────────────────────────────────────────────────────

BusListModel::BusListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

// ─────────────────────────────────────────────────────────────────────────────
// QAbstractListModel interface
// ─────────────────────────────────────────────────────────────────────────────

int BusListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return buses_.size();
}

QVariant BusListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= buses_.size())
        return {};

    const BusData& b = buses_.at(index.row());

    switch (static_cast<BusRole>(role))
    {
        case BusIdRole:        return b.busId;
        case ColorIndexRole:   return b.colorIndex;
        case CapacityRole:     return b.capacity;
        case BoardedCountRole: return b.boardedCount;
        case BoardRowRole:     return b.boardRow;
        case BoardColRole:     return b.boardCol;
        case DirectionEnumRole:return b.directionEnum;
        case IsActiveRole:     return b.isActive;
        case IsAtPlatformRole: return b.isAtPlatform;
        case HasLeftRole:      return b.hasLeft;
    }
    return {};
}

QHash<int, QByteArray> BusListModel::roleNames() const
{
    return {
        { BusIdRole,         "busId"         },
        { ColorIndexRole,    "colorIndex"    },
        { CapacityRole,      "capacity"      },
        { BoardedCountRole,  "boardedCount"  },
        { BoardRowRole,      "boardRow"      },
        { BoardColRole,      "boardCol"      },
        { DirectionEnumRole, "directionEnum" },
        { IsActiveRole,      "isActive"      },
        { IsAtPlatformRole,  "isAtPlatform"  },
        { HasLeftRole,       "hasLeft"       },
    };
}

// ─────────────────────────────────────────────────────────────────────────────
// Mutação do modelo
// ─────────────────────────────────────────────────────────────────────────────

void BusListModel::resetFromBoard(const Board& board)
{
    beginResetModel();

    buses_.clear();
    idToIndex_.clear();

    for (const auto& busPtr : board.buses())
    {
        idToIndex_.insert(busPtr->id(), buses_.size());
        buses_.append(fromBus(*busPtr));
    }

    endResetModel();

    qDebug().noquote()
        << "BusListModel: reset com" << buses_.size() << "autocarros.";
}

bool BusListModel::updateBus(int busId, int row, int col,
                              int boardedCount, bool isAtPlatform, bool hasLeft)
{
    const auto it = idToIndex_.find(busId);
    if (it == idToIndex_.end())
    {
        qWarning().noquote()
            << "BusListModel::updateBus: id" << busId << "não encontrado.";
        return false;
    }

    const int idx = it.value();
    BusData&  b   = buses_[idx];

    b.boardRow     = row;
    b.boardCol     = col;
    b.boardedCount = boardedCount;
    b.isAtPlatform = isAtPlatform;
    b.hasLeft      = hasLeft;
    b.isActive     = !hasLeft;

    const QModelIndex mi = index(idx);
    emit dataChanged(mi, mi, {
        BoardRowRole, BoardColRole, BoardedCountRole,
        IsAtPlatformRole, HasLeftRole, IsActiveRole
    });

    return true;
}

void BusListModel::clear()
{
    beginResetModel();
    buses_.clear();
    idToIndex_.clear();
    endResetModel();
}

// ─────────────────────────────────────────────────────────────────────────────
// Auxiliares privados
// ─────────────────────────────────────────────────────────────────────────────

BusListModel::BusData BusListModel::fromBus(const Bus& bus)
{
    BusData d;
    d.busId        = bus.id();
    d.colorIndex   = static_cast<int>(bus.color());
    d.capacity     = bus.capacity();
    d.boardedCount = bus.boardedCount();
    d.boardRow     = bus.row();
    d.boardCol     = bus.col();
    d.directionEnum= static_cast<int>(bus.direction());
    d.isActive     = bus.isActive();
    d.isAtPlatform = bus.isAtPlatform();
    d.hasLeft      = bus.hasLeft();
    return d;
}

void BusListModel::rebuildIdIndex()
{
    idToIndex_.clear();
    for (int i = 0; i < buses_.size(); ++i)
        idToIndex_.insert(buses_.at(i).busId, i);
}
