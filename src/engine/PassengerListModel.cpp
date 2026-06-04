#include "PassengerListModel.h"
#include <QDebug>

PassengerListModel::PassengerListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

int PassengerListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return passengers_.size();
}

QVariant PassengerListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= passengers_.size())
        return {};

    const PassengerData& p = passengers_.at(index.row());

    switch (static_cast<PassengerRole>(role))
    {
        case PassengerIdRole: return p.passengerId;
        case ColorIndexRole:  return p.colorIndex;
        case CellRowRole:     return p.cellRow;
        case CellColRole:     return p.cellCol;
        case StateEnumRole:   return p.stateEnum;
        case IsWaitingRole:   return p.isWaiting;
        case IsBoardingRole:  return p.isBoarding;
        case IsBoardedRole:   return p.isBoarded;
    }
    return {};
}

QHash<int, QByteArray> PassengerListModel::roleNames() const
{
    return {
        { PassengerIdRole, "passengerId" },
        { ColorIndexRole,  "colorIndex"  },
        { CellRowRole,     "cellRow"     },
        { CellColRole,     "cellCol"     },
        { StateEnumRole,   "stateEnum"   },
        { IsWaitingRole,   "isWaiting"   },
        { IsBoardingRole,  "isBoarding"  },
        { IsBoardedRole,   "isBoarded"   },
    };
}

void PassengerListModel::resetFromBoard(const Board& board)
{
    beginResetModel();
    passengers_.clear();
    idToIndex_.clear();

    for (const auto& pPtr : board.passengers())
    {
        idToIndex_.insert(pPtr->id(), passengers_.size());
        passengers_.append(fromPassenger(*pPtr));
    }

    endResetModel();
    qDebug().noquote()
        << "PassengerListModel: reset com" << passengers_.size() << "passageiros.";
}

bool PassengerListModel::updatePassenger(int passengerId, bool isBoarding, bool isBoarded)
{
    const auto it = idToIndex_.find(passengerId);
    if (it == idToIndex_.end())
    {
        qWarning().noquote()
            << "PassengerListModel::updatePassenger: id" << passengerId << "não encontrado.";
        return false;
    }

    const int idx = it.value();
    PassengerData& p = passengers_[idx];

    p.isBoarding = isBoarding;
    p.isBoarded  = isBoarded;
    p.isWaiting  = !isBoarding && !isBoarded;
    p.stateEnum  = isBoarded ? 2 : (isBoarding ? 1 : 0);

    const QModelIndex mi = index(idx);
    emit dataChanged(mi, mi, {
        StateEnumRole, IsWaitingRole, IsBoardingRole, IsBoardedRole
    });

    return true;
}

void PassengerListModel::clear()
{
    beginResetModel();
    passengers_.clear();
    idToIndex_.clear();
    endResetModel();
}

PassengerListModel::PassengerData
PassengerListModel::fromPassenger(const Passenger& p)
{
    PassengerData d;
    d.passengerId = p.id();
    d.colorIndex  = static_cast<int>(p.color());
    d.cellRow     = p.row();
    d.cellCol     = p.col();
    d.stateEnum   = static_cast<int>(p.state());
    d.isWaiting   = p.isWaiting();
    d.isBoarding  = p.isBoarding();
    d.isBoarded   = p.isBoarded();
    return d;
}
