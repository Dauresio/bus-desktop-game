/**
 * @file PassengerListModel.h
 * @brief QAbstractListModel para expor a lista de passageiros ao QML.
 *
 * Roles expostos:
 *  passengerId  int   — id único
 *  colorIndex   int   — BusColor enum
 *  cellRow      int   — linha na grelha
 *  cellCol      int   — coluna na grelha
 *  stateEnum    int   — PassengerState (0=Waiting,1=Boarding,2=Boarded)
 *  isWaiting    bool
 *  isBoarding   bool
 *  isBoarded    bool
 */

#pragma once

#include "Board.h"
#include <QAbstractListModel>
#include <QHash>

class PassengerListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum PassengerRole
    {
        PassengerIdRole = Qt::UserRole + 1,
        ColorIndexRole,
        CellRowRole,
        CellColRole,
        StateEnumRole,
        IsWaitingRole,
        IsBoardingRole,
        IsBoardedRole
    };
    Q_ENUM(PassengerRole)

    explicit PassengerListModel(QObject* parent = nullptr);

    [[nodiscard]] int      rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void resetFromBoard(const Board& board);

    /**
     * @brief Actualiza o estado de embarque de um passageiro.
     * @param passengerId Id do passageiro.
     * @param isBoarding  Embarque em curso.
     * @param isBoarded   Dentro do autocarro.
     */
    bool updatePassenger(int passengerId, bool isBoarding, bool isBoarded);

    void clear();

private:
    struct PassengerData
    {
        int  passengerId { -1 };
        int  colorIndex  { 0  };
        int  cellRow     { 0  };
        int  cellCol     { 0  };
        int  stateEnum   { 0  };
        bool isWaiting   { true  };
        bool isBoarding  { false };
        bool isBoarded   { false };
    };

    QList<PassengerData> passengers_;
    QHash<int, int>      idToIndex_;

    [[nodiscard]] static PassengerData fromPassenger(const Passenger& p);
};
