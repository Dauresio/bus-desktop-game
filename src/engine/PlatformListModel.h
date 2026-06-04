/**
 * @file PlatformListModel.h
 * @brief QAbstractListModel para expor a lista de plataformas ao QML.
 *
 * Roles expostos:
 *  platformId   int   — id único
 *  colorIndex   int   — BusColor enum
 *  platformRow  int   — linha (fixo)
 *  platformCol  int   — coluna (fixo)
 *  exitDirEnum  int   — Direction enum
 *  stateEnum    int   — PlatformState (0=Free,1=Occupied,2=Completed)
 *  isFree       bool
 *  isOccupied   bool
 *  isCompleted  bool
 */

#pragma once

#include "Board.h"
#include <QAbstractListModel>
#include <QHash>

class PlatformListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum PlatformRole
    {
        PlatformIdRole  = Qt::UserRole + 1,
        ColorIndexRole,
        PlatformRowRole,
        PlatformColRole,
        ExitDirEnumRole,
        StateEnumRole,
        IsFreeRole,
        IsOccupiedRole,
        IsCompletedRole
    };
    Q_ENUM(PlatformRole)

    explicit PlatformListModel(QObject* parent = nullptr);

    [[nodiscard]] int      rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void resetFromBoard(const Board& board);

    /**
     * @brief Actualiza o estado de uma plataforma.
     * @param platformId Id da plataforma.
     * @param stateEnum  Novo estado (0=Free,1=Occupied,2=Completed).
     */
    bool updatePlatform(int platformId, int stateEnum);

    void clear();

private:
    struct PlatformData
    {
        int  platformId  { -1 };
        int  colorIndex  { 0  };
        int  platformRow { 0  };
        int  platformCol { 0  };
        int  exitDirEnum { 3  };   // Right
        int  stateEnum   { 0  };   // Free
        bool isFree      { true  };
        bool isOccupied  { false };
        bool isCompleted { false };
    };

    QList<PlatformData> platforms_;
    QHash<int, int>     idToIndex_;

    [[nodiscard]] static PlatformData fromPlatform(const Platform& pl);
};
