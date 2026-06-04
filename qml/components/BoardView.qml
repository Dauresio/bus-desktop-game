/**
 * @file BoardView.qml
 * @brief Componente visual do tabuleiro de jogo completo.
 *
 * Responsabilidades:
 * - Renderizar a grelha de células (fundo xadrez cartoon)
 * - Instanciar PassengerItem para cada passageiro
 * - Instanciar BusItem para cada autocarro
 * - Instanciar PlatformView para cada plataforma
 * - Receber actualizações do GameController via sinais e reflectir
 *   na posição/estado dos componentes
 * - Propagar toque em autocarro para o GameController
 *
 * Modelo de dados:
 * O BoardView não acede directamente às classes C++ — recebe listas
 * de modelos JSON simples do GameController. Cada entrada contém os
 * campos necessários para renderizar a entidade.
 *
 * Exemplo de entrada no busModel:
 *   { "id": 0, "colorIndex": 0, "capacity": 4, "boardedCount": 2,
 *     "row": 3, "col": 1, "direction": 2, "isActive": true,
 *     "isAtPlatform": false, "hasLeft": false }
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import "../style" as Style

Item {
    id: root

    // ─── Propriedades públicas ─────────────────────────────────────────────

    property int    boardRows:  8
    property int    boardCols:  8
    property var    busModel:   []         // Array de objetos bus
    property var    passengerModel: []     // Array de objetos passenger
    property var    platformModel:  []     // Array de objetos platform

    // ─── Dimensões ─────────────────────────────────────────────────────────

    readonly property real cellSize: Style.Sizes.cellSize

    width:  cellSize * boardCols
    height: cellSize * boardRows

    // ─── Sinais ────────────────────────────────────────────────────────────

    signal busTapped(int busId)

    // ─── Fundo: grelha xadrez cartoon ─────────────────────────────────────

    Rectangle {
        id: boardBackground
        anchors.fill: parent
        color:   Style.Theme.bgBoard
        radius:  Style.Theme.radiusMedium
        border.color: Style.Theme.borderDark
        border.width: Style.Theme.borderWidthHeavy

        // Sombra do tabuleiro
        layer.enabled: true
        layer.effect: null   // Sem MultiEffect por portabilidade; sombra manual abaixo
    }

    // Sombra manual do tabuleiro
    Rectangle {
        width:  boardBackground.width
        height: boardBackground.height
        radius: Style.Theme.radiusMedium
        color:  Style.Theme.shadowColor
        opacity: 0.5
        x: Style.Theme.shadowOffset * 2
        y: Style.Theme.shadowOffset * 2
        z: -1
    }

    // Grelha de células
    Grid {
        id: cellGrid
        anchors.fill: parent
        rows:    boardRows
        columns: boardCols

        Repeater {
            model: boardRows * boardCols

            Rectangle {
                width:  cellSize
                height: cellSize

                // Xadrez: alterna entre duas tonalidades de azul
                readonly property int row: Math.floor(index / boardCols)
                readonly property int col: index % boardCols
                readonly property bool isEven: (row + col) % 2 === 0

                color: isEven ? Style.Theme.bgCell : Style.Theme.bgCellAlt

                // Borda subtil entre células
                border.color: "#18FFFFFF"
                border.width: 1
            }
        }
    }

    // ─── Plataformas ──────────────────────────────────────────────────────

    Repeater {
        id: platformRepeater
        model: platformModel

        delegate: PlatformView {
            platformId:  model.platformId
            colorIndex:  model.colorIndex
            stateEnum:   model.stateEnum
            exitDirEnum: model.exitDirEnum
            cellSize:    root.cellSize

            x: model.platformCol * cellSize
            y: model.platformRow * cellSize
        }
    }

    // ─── Passageiros ──────────────────────────────────────────────────────

    Repeater {
        id: passengerRepeater
        model: passengerModel

        delegate: PassengerItem {
            passengerId: model.passengerId
            colorIndex:  model.colorIndex
            isBoarding:  model.isBoarding
            isBoarded:   model.isBoarded
            cellSize:    root.cellSize

            x: model.cellCol * cellSize
            y: model.cellRow * cellSize
        }
    }

    // ─── Autocarros ───────────────────────────────────────────────────────

    Repeater {
        id: busRepeater
        model: busModel

        delegate: BusItem {
            busId:        model.busId
            colorIndex:   model.colorIndex
            capacity:     model.capacity
            boardedCount: model.boardedCount
            isActive:     model.isActive
            isAtPlatform: model.isAtPlatform
            hasLeft:      model.hasLeft
            directionEnum: model.directionEnum
            cellSize:     root.cellSize
            boardRow:     model.boardRow
            boardCol:     model.boardCol

            onBusTapped: function(id) {
                root.busTapped(id)
            }
        }
    }

    // ─── Overlay de célula seleccionada (feedback de hint) ────────────────

    property int hintBusId: -1

    Repeater {
        model: busModel

        delegate: Rectangle {
            visible: model.busId === root.hintBusId && root.hintBusId >= 0
            width:  cellSize * Style.Theme.busLengthFactor(model.capacity)
            height: cellSize * 0.82
            radius: Style.Theme.radiusBus
            color:  "transparent"
            border.color: "#FFFFD700"
            border.width: 3
            x: model.boardCol * cellSize + (cellSize - width) / 2
            y: model.boardRow * cellSize + (cellSize - height) / 2

            // Pulso do hint
            SequentialAnimation on opacity {
                running: visible
                loops: Animation.Infinite
                NumberAnimation { to: 0.3; duration: 500 }
                NumberAnimation { to: 1.0; duration: 500 }
            }
        }
    }

    // ─── Efeito de vitória (partículas simples) ───────────────────────────

    property bool showVictoryEffect: false

    Repeater {
        model: showVictoryEffect ? 12 : 0

        delegate: Rectangle {
            id: particle
            width:  8 + index * 2
            height: width
            radius: width / 2
            color: ["#E74C3C","#3498DB","#2ECC71","#F1C40F","#9B59B6"][index % 5]

            // Posição inicial aleatória no centro do tabuleiro
            x: root.width  * 0.3 + Math.random() * root.width  * 0.4
            y: root.height * 0.3 + Math.random() * root.height * 0.4

            ParallelAnimation {
                running: true

                NumberAnimation {
                    target: particle; property: "y"
                    to: -40 - index * 20
                    duration: 800 + index * 80
                    easing.type: Easing.OutQuad
                }
                NumberAnimation {
                    target: particle; property: "opacity"
                    from: 1.0; to: 0.0
                    duration: 800 + index * 80
                }
                NumberAnimation {
                    target: particle; property: "x"
                    to: particle.x + (index % 2 === 0 ? 1 : -1) * (20 + index * 10)
                    duration: 800 + index * 80
                    easing.type: Easing.OutSine
                }
            }
        }
    }


}
