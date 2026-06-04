#include "PlatformListModel.h"
#include <QDebug>

PlatformListModel::PlatformListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

int PlatformListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return platforms_.size();
}

QVariant PlatformListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= platforms_.size())
        return {};

    const PlatformData& pl = platforms_.at(index.row());

    switch (static_cast<PlatformRole>(role))
    {
        case PlatformIdRole:  return pl.platformId;
        case ColorIndexRole:  return pl.colorIndex;
        case PlatformRowRole: return pl.platformRow;
        case PlatformColRole: return pl.platformCol;
        case ExitDirEnumRole: return pl.exitDirEnum;
        case StateEnumRole:   return pl.stateEnum;
        case IsFreeRole:      return pl.isFree;
        case IsOccupiedRole:  return pl.isOccupied;
        case IsCompletedRole: return pl.isCompleted;
    }
    return {};
}

QHash<int, QByteArray> PlatformListModel::roleNames() const
{
    return {
        { PlatformIdRole,  "platformId"  },
        { ColorIndexRole,  "colorIndex"  },
        { PlatformRowRole, "platformRow" },
        { PlatformColRole, "platformCol" },
        { ExitDirEnumRole, "exitDirEnum" },
        { StateEnumRole,   "stateEnum"   },
        { IsFreeRole,      "isFree"      },
        { IsOccupiedRole,  "isOccupied"  },
        { IsCompletedRole, "isCompleted" },
    };
}

void PlatformListModel::resetFromBoard(const Board& board)
{
    beginResetModel();
    platforms_.clear();
    idToIndex_.clear();

    for (const auto& plPtr : board.platforms())
    {
        idToIndex_.insert(plPtr->id(), platforms_.size());
        platforms_.append(fromPlatform(*plPtr));
    }

    endResetModel();
    qDebug().noquote()
        << "PlatformListModel: reset com" << platforms_.size() << "plataformas.";
}

bool PlatformListModel::updatePlatform(int platformId, int stateEnum)
{
    const auto it = idToIndex_.find(platformId);
    if (it == idToIndex_.end())
    {
        qWarning().noquote()
            << "PlatformListModel::updatePlatform: id" << platformId << "não encontrado.";
        return false;
    }

    const int idx   = it.value();
    PlatformData& pl = platforms_[idx];

    pl.stateEnum   = stateEnum;
    pl.isFree      = (stateEnum == 0);
    pl.isOccupied  = (stateEnum == 1);
    pl.isCompleted = (stateEnum == 2);

    const QModelIndex mi = index(idx);
    emit dataChanged(mi, mi, {
        StateEnumRole, IsFreeRole, IsOccupiedRole, IsCompletedRole
    });

    return true;
}

void PlatformListModel::clear()
{
    beginResetModel();
    platforms_.clear();
    idToIndex_.clear();
    endResetModel();
}

PlatformListModel::PlatformData
PlatformListModel::fromPlatform(const Platform& pl)
{
    PlatformData d;
    d.platformId  = pl.id();
    d.colorIndex  = static_cast<int>(pl.color());
    d.platformRow = pl.row();
    d.platformCol = pl.col();
    d.exitDirEnum = static_cast<int>(pl.exitDirection());
    d.stateEnum   = static_cast<int>(pl.state());
    d.isFree      = pl.isFree();
    d.isOccupied  = pl.isOccupied();
    d.isCompleted = pl.isCompleted();
    return d;
}
