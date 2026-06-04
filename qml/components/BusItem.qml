/**
 * @file BusItem.qml
 * @brief Componente visual de um autocarro no tabuleiro.
 *
 * Renderiza um autocarro cartoon com:
 * - Corpo proporcional à capacidade (4=1 célula, 6=1.5, 8=2, 12=3)
 * - Janelas, rodas, faróis
 * - Bordo grosso e sombra cartoon
 * - Animação de movimento suave (PathAnimation célula a célula)
 * - Animação de chegada à plataforma (bounce + brilho)
 * - Animação de saída do tabuleiro (slide out)
 * - Indicador de toque (highlight ao pressionar)
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import "../style" as Style

Item {
    id: root

    // ─── Propriedades públicas ─────────────────────────────────────────────

    property int    busId:       -1
    property int    colorIndex:  0       // BusColor enum (0-4)
    property int    capacity:    4       // 4, 6, 8 ou 12
    property int    boardedCount: 0
    property bool   isActive:    true
    property bool   isAtPlatform: false
    property bool   hasLeft:     false
    property int    directionEnum: 2     // Direction::Right = 2
    property real   cellSize:    Style.Sizes.cellSize

    // Posição no tabuleiro (em células) — actualizada pelo BoardView
    property int    boardRow:    0
    property int    boardCol:    0

    // ─── Dimensões derivadas ───────────────────────────────────────────────

    // Comprimento do autocarro em pixels (proporcional à capacidade)
    readonly property real busLength: cellSize * Style.Theme.busLengthFactor(capacity)
    readonly property real busH:      Style.Sizes.busHeight

    // Orientação: horizontal (Right/Left) ou vertical (Up/Down)
    readonly property bool isHorizontal: directionEnum === 2 || directionEnum === 3

    width:  isHorizontal ? busLength : busH
    height: isHorizontal ? busH      : busLength

    // ─── Posicionamento no tabuleiro ──────────────────────────────────────

    x: boardCol * cellSize + (cellSize - width)  / 2
    y: boardRow * cellSize + (cellSize - height) / 2

    Behavior on x {
        enabled: !hasLeft
        SmoothedAnimation {
            velocity: cellSize * 8   // 8 células por segundo
            easing.type: Easing.OutQuad
        }
    }
    Behavior on y {
        enabled: !hasLeft
        SmoothedAnimation {
            velocity: cellSize * 8
            easing.type: Easing.OutQuad
        }
    }

    visible: isActive && !hasLeft

    // ─── Sombra cartoon ───────────────────────────────────────────────────

    Rectangle {
        id: shadow
        width:  parent.width
        height: parent.height
        radius: Style.Theme.radiusBus + 2
        color:  Style.Theme.shadowColor
        opacity: Style.Theme.shadowOpacity
        x: Style.Theme.shadowOffset
        y: Style.Theme.shadowOffset
        rotation: bodyRect.rotation
    }

    // ─── Corpo principal ───────────────────────────────────────────────────

    Rectangle {
        id: bodyRect
        anchors.fill: parent
        radius: Style.Theme.radiusBus
        color:  Style.Theme.busColorFill(colorIndex)
        border.color: Style.Theme.busColorStroke(colorIndex)
        border.width: Style.Theme.borderWidthHeavy
        rotation: isHorizontal ? 0 : 0  // Rotação tratada via swap width/height

        // Faixa lateral de cor mais clara (detalhe cartoon)
        Rectangle {
            id: topStripe
            width:  parent.width
            height: parent.height * 0.28
            radius: Style.Theme.radiusBus
            color:  Style.Theme.busColorLight(colorIndex)
            opacity: 0.6
            anchors.top: parent.top
            // Recorta os cantos inferiores
            Rectangle {
                width:  parent.width
                height: parent.height * 0.5
                color:  parent.color
                opacity: parent.opacity
                anchors.bottom: parent.bottom
            }
        }

        // ── Janelas (horizontal) ──────────────────────────────────────────

        Row {
            id: windowsRow
            visible: isHorizontal
            anchors {
                top:    parent.top
                topMargin: parent.height * 0.1
                left:   parent.left
                leftMargin: parent.width * 0.15
                right:  parent.right
                rightMargin: parent.width * 0.1
            }
            height: parent.height * 0.42
            spacing: parent.width * 0.06

            // Número de janelas proporcional ao comprimento
            Repeater {
                model: Math.max(1, Math.floor(capacity / 3))
                Rectangle {
                    width:  (windowsRow.width - windowsRow.spacing * (model-1)) / model
                    height: windowsRow.height
                    radius: Style.Theme.radiusSmall
                    color:  "#99D6F5"    // Vidro azul cartoon
                    border.color: Style.Theme.borderDark
                    border.width: 2
                    // Reflexo da janela
                    Rectangle {
                        width:  parent.width * 0.3
                        height: parent.height * 0.4
                        radius: 2
                        color:  "#60FFFFFF"
                        anchors { top: parent.top; topMargin: 3; left: parent.left; leftMargin: 3 }
                    }
                }
            }
        }

        // ── Faróis (direito) ──────────────────────────────────────────────

        Rectangle {
            visible: isHorizontal && directionEnum === 2  // Right
            width:   parent.height * 0.22
            height:  parent.height * 0.22
            radius:  width / 2
            color:   "#FFF176"
            border.color: Style.Theme.borderDark
            border.width: 2
            anchors {
                right:  parent.right
                rightMargin: parent.height * 0.12
                verticalCenter: parent.verticalCenter
            }
        }

        // ── Faróis (esquerdo) ─────────────────────────────────────────────

        Rectangle {
            visible: isHorizontal && directionEnum === 3  // Left
            width:   parent.height * 0.22
            height:  parent.height * 0.22
            radius:  width / 2
            color:   "#FFF176"
            border.color: Style.Theme.borderDark
            border.width: 2
            anchors {
                left:  parent.left
                leftMargin: parent.height * 0.12
                verticalCenter: parent.verticalCenter
            }
        }

        // ── Rodas ─────────────────────────────────────────────────────────

        // Roda dianteira (horizontal)
        Rectangle {
            visible: isHorizontal
            width:   parent.height * 0.28
            height:  parent.height * 0.28
            radius:  width / 2
            color:   "#2C3E50"
            border.color: Style.Theme.borderDark
            border.width: 2
            anchors {
                bottom: parent.bottom
                bottomMargin: -height * 0.35
                left:   parent.left
                leftMargin: parent.width * 0.15
            }
            // Aro
            Rectangle {
                width: parent.width * 0.45; height: width
                radius: width/2; color: "#7F8C8D"
                anchors.centerIn: parent
            }
        }

        // Roda traseira (horizontal)
        Rectangle {
            visible: isHorizontal
            width:   parent.height * 0.28
            height:  parent.height * 0.28
            radius:  width / 2
            color:   "#2C3E50"
            border.color: Style.Theme.borderDark
            border.width: 2
            anchors {
                bottom: parent.bottom
                bottomMargin: -height * 0.35
                right:  parent.right
                rightMargin: parent.width * 0.15
            }
            Rectangle {
                width: parent.width * 0.45; height: width
                radius: width/2; color: "#7F8C8D"
                anchors.centerIn: parent
            }
        }

        // ── Indicador de passageiros (barra de progresso interior) ────────

        Rectangle {
            id: capacityBar
            visible: boardedCount > 0
            anchors {
                bottom: parent.bottom
                bottomMargin: 4
                left: parent.left
                leftMargin: 6
                right: parent.right
                rightMargin: 6
            }
            height: 5
            radius: height / 2
            color: Style.Theme.borderDark
            opacity: 0.5

            Rectangle {
                width:  parent.width * (boardedCount / capacity)
                height: parent.height
                radius: parent.radius
                color:  "#FFFFFF"
                Behavior on width {
                    NumberAnimation { duration: Style.Theme.animFast }
                }
            }
        }
    }

    // ─── Efeito de toque / pressed ────────────────────────────────────────

    Rectangle {
        id: pressOverlay
        anchors.fill: bodyRect
        radius: Style.Theme.radiusBus
        color: "#40FFFFFF"
        visible: false
    }

    // ─── Área de toque ────────────────────────────────────────────────────

    MouseArea {
        id: touchArea
        anchors.fill: parent
        // Expande a área de toque em mobile
        anchors.margins: Style.Sizes.isMobile ? -6 : 0

        onPressed: {
            pressOverlay.visible = true
            root.scale = 0.96
        }
        onReleased: {
            pressOverlay.visible = false
            root.scale = 1.0
            busTapped(busId)
        }
        onCanceled: {
            pressOverlay.visible = false
            root.scale = 1.0
        }
    }

    signal busTapped(int id)

    // ─── Animação de bounce ao chegar à plataforma ────────────────────────

    SequentialAnimation {
        id: platformArrivalAnim
        running: isAtPlatform

        NumberAnimation {
            target: root; property: "scale"
            to: 1.1; duration: 120; easing.type: Easing.OutQuad
        }
        NumberAnimation {
            target: root; property: "scale"
            to: 1.0; duration: 180; easing.type: Easing.OutBounce
        }
    }

    // ─── Animação de saída do tabuleiro ───────────────────────────────────

    ParallelAnimation {
        id: exitAnim
        running: hasLeft

        NumberAnimation {
            target: root; property: "opacity"
            to: 0.0; duration: Style.Theme.animMedium
        }
        NumberAnimation {
            target: root; property: "scale"
            to: 0.7; duration: Style.Theme.animMedium; easing.type: Easing.InBack
        }
    }

    // ─── Comportamento de escala global ───────────────────────────────────

    Behavior on scale {
        NumberAnimation { duration: Style.Theme.animFast; easing.type: Easing.OutBack }
    }
}
