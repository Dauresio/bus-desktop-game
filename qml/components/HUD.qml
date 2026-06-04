/**
 * @file HUD.qml
 * @brief Heads-Up Display superior do jogo.
 *
 * Exibe em tempo real:
 * - Número de movimentos
 * - Tempo decorrido / restante
 * - Score atual
 * - Botões: Pause, Restart, Hint, Undo
 *
 * Responsivo: em mobile usa ícones+números compactos;
 * em desktop mostra labels completos.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../style" as Style

Rectangle {
    id: root

    // ─── Propriedades públicas ─────────────────────────────────────────────

    property int  moveCount:         0
    property int  elapsedSeconds:    0
    property int  remainingSeconds:  0
    property int  currentScore:      0
    property int  remainingPassengers: 0
    property bool hasTimeLimit:      false
    property bool canUndo:           false
    property bool canHint:           false

    // ─── Sinais ────────────────────────────────────────────────────────────

    signal pauseRequested()
    signal restartRequested()
    signal undoRequested()
    signal hintRequested()

    // ─── Estilo cartoon ────────────────────────────────────────────────────

    color:  Style.Theme.bgHUD
    height: Style.Sizes.hudHeight

    // Bordo inferior cartoon
    Rectangle {
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: Style.Theme.borderWidthHeavy
        color:  Style.Theme.borderDark
    }

    // Sombra inferior
    Rectangle {
        anchors { top: parent.bottom; left: parent.left; right: parent.right }
        height: 6
        opacity: 0.3
        color: Style.Theme.shadowColor
    }

    // ─── Layout principal ─────────────────────────────────────────────────

    RowLayout {
        anchors {
            fill: parent
            leftMargin:  Style.Sizes.spaceMD
            rightMargin: Style.Sizes.spaceMD
            topMargin:   Style.Sizes.spaceXS
            bottomMargin: Style.Sizes.spaceXS
        }
        spacing: Style.Sizes.spaceMD

        // ── Botão Pause ───────────────────────────────────────────────────
        HudButton {
            icon: "⏸"
            onClicked: root.pauseRequested()
        }

        // ── Score ─────────────────────────────────────────────────────────
        HudMetric {
            label: "★"
            value: root.currentScore
            valueColor: Style.Theme.textScore
            Layout.fillWidth: true
        }

        // ── Movimentos ────────────────────────────────────────────────────
        HudMetric {
            label: Style.Sizes.isMobile ? "↕" : "Jogadas"
            value: root.moveCount
            Layout.fillWidth: true
        }

        // ── Tempo ─────────────────────────────────────────────────────────
        HudMetric {
            label: Style.Sizes.isMobile ? "⏱" : "Tempo"
            value: root.hasTimeLimit ? root.remainingSeconds : root.elapsedSeconds
            valueColor: (root.hasTimeLimit && root.remainingSeconds < 15)
                        ? Style.Theme.textLose
                        : Style.Theme.textPrimary
            isTime: true
            Layout.fillWidth: true

            // Pulso de aviso quando < 15 segundos
            SequentialAnimation on opacity {
                running: root.hasTimeLimit && root.remainingSeconds < 15 && root.remainingSeconds > 0
                loops: Animation.Infinite
                NumberAnimation { to: 0.4; duration: 400 }
                NumberAnimation { to: 1.0; duration: 400 }
            }
        }

        // ── Undo ──────────────────────────────────────────────────────────
        HudButton {
            icon:    "↩"
            enabled: root.canUndo
            onClicked: root.undoRequested()
        }

        // ── Hint ──────────────────────────────────────────────────────────
        HudButton {
            icon:    "💡"
            enabled: root.canHint
            onClicked: root.hintRequested()
        }

        // ── Restart ───────────────────────────────────────────────────────
        HudButton {
            icon: "↺"
            onClicked: root.restartRequested()
        }
    }

    // ─── Componentes internos ─────────────────────────────────────────────

    component HudButton: Rectangle {
        property string icon: ""

        signal clicked()

        width:  Style.Sizes.buttonHeight
        height: Style.Sizes.buttonHeight
        radius: Style.Sizes.buttonRadius
        color:  enabled ? (mouseArea.pressed ? Style.Theme.bgButtonHov : Style.Theme.bgButton)
                        : Style.Theme.bgButtonDis
        border.color: Style.Theme.borderDark
        border.width: Style.Theme.borderWidth
        opacity: enabled ? 1.0 : 0.5

        // Sombra
        Rectangle {
            width: parent.width; height: parent.height
            radius: parent.radius
            color: Style.Theme.shadowColor
            opacity: Style.Theme.shadowOpacity
            x: 3; y: 3; z: -1
        }

        Text {
            anchors.centerIn: parent
            text: icon
            font.pixelSize: Style.Sizes.fontSizeLG
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            enabled: parent.enabled
            onClicked: parent.clicked()
        }

        Behavior on color {
            ColorAnimation { duration: Style.Theme.animFast }
        }
        Behavior on scale {
            NumberAnimation { duration: 80 }
        }
        scale: mouseArea.pressed ? 0.93 : 1.0
    }

    component HudMetric: Column {
        property string label:      ""
        property int    value:      0
        property color  valueColor: Style.Theme.textPrimary
        property bool   isTime:     false

        spacing: -2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text:  parent.label
            color: "#AAFFFFFF"
            font {
                pixelSize: Style.Sizes.fontSizeSM
                family:    Style.Theme.fontBody
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: {
                if (!parent.isTime) return parent.value.toString()
                var s = parent.value
                var m = Math.floor(s / 60)
                var sec = s % 60
                return m.toString() + ":" + (sec < 10 ? "0" : "") + sec.toString()
            }
            color: parent.valueColor
            font {
                pixelSize: Style.Sizes.fontSizeMD
                family:    Style.Theme.fontDisplay
                bold:      true
            }
            Behavior on color { ColorAnimation { duration: Style.Theme.animFast } }
        }
    }
}
