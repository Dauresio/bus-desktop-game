/**
 * @file BusListModel.h
 * @brief QAbstractListModel para expor a lista de autocarros ao QML.
 *
 * O BoardView em QML usa:
 *   Repeater { model: gameController.busModel }
 *
 * Cada item do modelo expõe os seguintes roles:
 *
 *  busId         int   — id único do autocarro
 *  colorIndex    int   — BusColor enum (0-4)
 *  capacity      int   — 4, 6, 8 ou 12
 *  boardedCount  int   — passageiros embarcados
 *  boardRow      int   — linha actual na grelha
 *  boardCol      int   — coluna actual na grelha
 *  directionEnum int   — Direction enum (0=Up,1=Down,2=Left,3=Right)
 *  isActive      bool  — está no tabuleiro
 *  isAtPlatform  bool  — está sobre uma plataforma
 *  hasLeft       bool  — já saiu do tabuleiro
 *
 * ### Actualização incremental:
 *  Em vez de emitir dataChanged() para o modelo inteiro após cada jogada,
 *  updateBus() emite apenas para o índice do autocarro afectado — O(1)
 *  independentemente do número de autocarros.
 *
 * ### Thread-safety:
 *  Não thread-safe. Todas as chamadas na thread principal.
 */

#pragma once

#include "Board.h"

#include <QAbstractListModel>
#include <QHash>

class BusListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // ─── Roles expostos ao QML ────────────────────────────────────────────

    enum BusRole
    {
        BusIdRole        = Qt::UserRole + 1,
        ColorIndexRole,
        CapacityRole,
        BoardedCountRole,
        BoardRowRole,
        BoardColRole,
        DirectionEnumRole,
        IsActiveRole,
        IsAtPlatformRole,
        HasLeftRole
    };
    Q_ENUM(BusRole)

    // ─── Construção ───────────────────────────────────────────────────────

    explicit BusListModel(QObject* parent = nullptr);

    // ─── QAbstractListModel interface ─────────────────────────────────────

    [[nodiscard]] int      rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    // ─── Mutação do modelo ────────────────────────────────────────────────

    /**
     * @brief Repopula o modelo completo a partir de um Board.
     *
     * Chamado quando um novo nível é carregado.
     * Emite beginResetModel/endResetModel.
     *
     * @param board Board de origem.
     */
    void resetFromBoard(const Board& board);

    /**
     * @brief Actualiza o estado de um autocarro específico.
     *
     * Emite dataChanged() apenas para o índice afectado.
     * Chamado pelo GameController após busMoved, busLeft, etc.
     *
     * @param busId       Id do autocarro.
     * @param row         Nova linha.
     * @param col         Nova coluna.
     * @param boardedCount Passageiros embarcados.
     * @param isAtPlatform Está sobre plataforma.
     * @param hasLeft     Saiu do tabuleiro.
     * @return true se o autocarro foi encontrado e actualizado.
     */
    bool updateBus(int busId, int row, int col,
                   int boardedCount, bool isAtPlatform, bool hasLeft);

    /**
     * @brief Limpa o modelo (usado ao sair de um nível).
     */
    void clear();

private:
    // ─── Estrutura de dados interna ───────────────────────────────────────

    struct BusData
    {
        int  busId        { -1 };
        int  colorIndex   { 0  };
        int  capacity     { 4  };
        int  boardedCount { 0  };
        int  boardRow     { 0  };
        int  boardCol     { 0  };
        int  directionEnum{ 3  };  // Right
        bool isActive     { true  };
        bool isAtPlatform { false };
        bool hasLeft      { false };
    };

    QList<BusData>  buses_;
    QHash<int, int> idToIndex_;   ///< busId → índice em buses_

    // ─── Auxiliares ───────────────────────────────────────────────────────

    [[nodiscard]] static BusData fromBus(const Bus& bus);
    void rebuildIdIndex();
};
