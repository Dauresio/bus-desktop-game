/**
 * @file PlatformView.qml
 * @brief Componente visual de uma plataforma de embarque.
 *
 * Renderiza uma plataforma como um quadrado cartoon com:
 * - Anel de cor da plataforma (indica qual autocarro espera)
 * - Ícone de seta (indica direção de saída)
 * - Estado visual: Free (anel simples), Occupied (pulsação), Completed (check)
 * - Animação de pulso quando Occupied
 */

import QtQuick 2.15
import "../style" as Style

Item {
    id: root

    // ─── Propriedades públicas ─────────────────────────────────────────────

    property int    platformId:      -1
    property int    colorIndex:      0       // BusColor enum (0-4)
    property int    stateEnum:       0       // PlatformState: Free=0, Occupied=1, Completed=2
    property int    exitDirEnum:     2       // Direction enum: Right=2
    property real   cellSize:        Style.Sizes.cellSize

    // ─── Dimensões ─────────────────────────────────────────────────────────

    width:  cellSize
    height: cellSize

    // ─── Fundo da célula de plataforma ────────────────────────────────────

    Rectangle {
        id: bg
        anchors.fill: parent
        color:  "#0F3460"
        radius: 4
        opacity: 0.8
    }

    // ─── Anel externo da plataforma ───────────────────────────────────────

    Rectangle {
        id: outerRing
        width:  Style.Sizes.platformSize
        height: Style.Sizes.platformSize
        radius: Style.Theme.radiusMedium
        anchors.centerIn: parent
        color:  "transparent"
        border.color: Style.Theme.busColorFill(colorIndex)
        border.width: Style.Sizes.platformRingWidth + 1

        // Sombra do anel
        Rectangle {
            width: parent.width; height: parent.height
            radius: parent.radius
            color: "transparent"
            border.color: Style.Theme.busColorStroke(colorIndex)
            border.width: 2
            x: 2; y: 2
            z: -1
        }
    }

    // ─── Interior da plataforma ───────────────────────────────────────────

    Rectangle {
        id: innerArea
        width:  Style.Sizes.platformSize - Style.Sizes.platformRingWidth * 2 - 4
        height: width
        radius: Style.Theme.radiusSmall
        anchors.centerIn: parent
        color: stateEnum === 2  // Completed
               ? Style.Theme.busColorFill(colorIndex)
               : Style.Theme.bgBoard
        opacity: stateEnum === 2 ? 0.3 : 1.0

        Behavior on color   { ColorAnimation   { duration: Style.Theme.animMedium } }
        Behavior on opacity { NumberAnimation  { duration: Style.Theme.animMedium } }
    }

    // ─── Seta de direção ──────────────────────────────────────────────────

    Text {
        id: arrowLabel
        anchors.centerIn: parent
        text: {
            switch(exitDirEnum) {
                case 0: return "↑"   // Up
                case 1: return "↓"   // Down
                case 3: return "←"   // Left
                default: return "→"  // Right
            }
        }
        font.pixelSize: cellSize * 0.38
        font.family:    Style.Theme.fontDisplay
        color: stateEnum === 2
               ? Style.Theme.busColorFill(colorIndex)
               : Style.Theme.busColorFill(colorIndex)
        opacity: stateEnum === 2 ? 0.5 : 0.9
    }

    // ─── Check mark (Completed) ───────────────────────────────────────────

    Text {
        id: checkMark
        anchors.centerIn: parent
        text: "✓"
        font.pixelSize: cellSize * 0.42
        font.bold: true
        color: Style.Theme.busColorFill(colorIndex)
        visible: stateEnum === 2
        opacity: 0.0

        NumberAnimation on opacity {
            running: stateEnum === 2
            from: 0.0; to: 1.0
            duration: Style.Theme.animSlow
            easing.type: Easing.OutBack
        }
    }

    // ─── Animação de pulso (Occupied) ─────────────────────────────────────

    SequentialAnimation {
        id: pulseAnim
        running: stateEnum === 1   // Occupied
        loops:   Animation.Infinite

        NumberAnimation {
            target: outerRing; property: "scale"
            to: 1.08; duration: 600; easing.type: Easing.InOutSine
        }
        NumberAnimation {
            target: outerRing; property: "scale"
            to: 1.0;  duration: 600; easing.type: Easing.InOutSine
        }
    }

    // Reset da escala quando sai de Occupied
    onStateEnumChanged: {
        if (stateEnum !== 1)
            outerRing.scale = 1.0
    }

    // ─── Tooltip de cor (legenda) ─────────────────────────────────────────

    // Pequeno badge de cor no canto superior direito
    Rectangle {
        width:  cellSize * 0.22
        height: cellSize * 0.22
        radius: width / 2
        color:  Style.Theme.busColorFill(colorIndex)
        border.color: Style.Theme.borderDark
        border.width: 2
        anchors {
            top:   parent.top
            right: parent.right
            margins: 2
        }
    }
}
