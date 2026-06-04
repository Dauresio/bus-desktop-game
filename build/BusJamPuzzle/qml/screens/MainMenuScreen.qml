/**
 * @file MainMenuScreen.qml
 * @brief Ecrã de menu principal do Bus Jam Puzzle.
 *
 * Estilo cartoon vibrante: título animado, botões com bounce,
 * autocarros decorativos a circular no fundo.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import "../style" as Style

Item {
    id: root

    signal playRequested()
    signal settingsRequested()
    signal quitRequested()

    // ─── Fundo ────────────────────────────────────────────────────────────

    Rectangle {
        anchors.fill: parent
        color: Style.Theme.bgPage
    }

    // Círculos decorativos de fundo
    Repeater {
        model: 6
        Rectangle {
            width:  80 + index * 40
            height: width
            radius: width / 2
            color:  ["#E74C3C","#3498DB","#2ECC71","#F1C40F","#9B59B6","#E94560"][index]
            opacity: 0.06
            x: [20, root.width-160, root.width*0.5, 40, root.width-120, root.width*0.3][index]
            y: [40, 60, root.height-180, root.height-120, root.height*0.4, 20][index]
        }
    }

    // ─── Autocarros decorativos animados ──────────────────────────────────

    Repeater {
        model: 4

        Item {
            id: decoBus
            readonly property color busCol:
                [Style.Theme.busColorFill(0),
                 Style.Theme.busColorFill(1),
                 Style.Theme.busColorFill(2),
                 Style.Theme.busColorFill(3)][index]

            width:  90 + index * 10
            height: 38
            y: 60 + index * (root.height / 5)
            x: -width

            // Corpo
            Rectangle {
                anchors.fill: parent
                radius: 10
                color: decoBus.busCol
                border.color: Style.Theme.borderDark
                border.width: 3

                Rectangle {
                    width: parent.width * 0.55
                    height: parent.height * 0.38
                    radius: 5
                    color: "#99D6F5"
                    border.color: Style.Theme.borderDark
                    border.width: 2
                    anchors { top: parent.top; topMargin: 4; left: parent.left; leftMargin: 10 }
                }
            }

            // Animação de travessia
            NumberAnimation on x {
                running: true
                loops: Animation.Infinite
                from: -decoBus.width - 10
                to:   root.width + 10
                duration: 6000 + index * 1200
            }
        }
    }

    // ─── Conteúdo central ─────────────────────────────────────────────────

    Column {
        anchors.centerIn: parent
        spacing: Style.Sizes.spaceLG

        // Título principal
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Style.Sizes.spaceXS

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "🚌 BUS JAM"
                color: Style.Theme.textPrimary
                font {
                    family:    Style.Theme.fontDisplay
                    pixelSize: Style.Sizes.fontSizeXXL
                    bold:      false
                }
                // Bounce de entrada
                NumberAnimation on y {
                    running: true
                    from: -60; to: 0
                    duration: Style.Theme.animBounce
                    easing.type: Easing.OutBack
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "PUZZLE"
                color: Style.Theme.bgHUD
                font {
                    family:    Style.Theme.fontDisplay
                    pixelSize: Style.Sizes.fontSizeXL
                }
            }
        }

        // Separador colorido
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 120; height: 5; radius: 3
            color: Style.Theme.bgButton
            border.color: Style.Theme.borderDark; border.width: 2
        }

        // Botões
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Style.Sizes.spaceMD

            MenuButton {
                text:    "▶  JOGAR"
                bgColor: Style.Theme.textWin
                onClicked: root.playRequested()
            }

            MenuButton {
                text:    "⚙  DEFINIÇÕES"
                bgColor: Style.Theme.bgButton
                onClicked: root.settingsRequested()
            }

            MenuButton {
                text:    "✕  SAIR"
                bgColor: Style.Theme.bgHUD
                onClicked: root.quitRequested()
            }
        }

        // Versão
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "v1.0"
            color: "#40FFFFFF"
            font { family: Style.Theme.fontBody; pixelSize: Style.Sizes.fontSizeSM }
        }
    }

    // ─── Componente de botão de menu ──────────────────────────────────────

    component MenuButton: Rectangle {
        property string text:    ""
        property color  bgColor: Style.Theme.bgButton
        signal clicked()

        width:  220
        height: 52
        radius: Style.Sizes.buttonRadius
        color:  bgColor
        border.color: Style.Theme.borderDark
        border.width: Style.Theme.borderWidthHeavy

        Rectangle {
            width: parent.width; height: parent.height
            radius: parent.radius
            color: Style.Theme.shadowColor; opacity: Style.Theme.shadowOpacity
            x: 4; y: 4; z: -1
        }

        Text {
            anchors.centerIn: parent
            text: parent.text
            color: Style.Theme.textPrimary
            font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeMD }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: parent.clicked()
            onPressed: parent.y += 3
            onReleased: parent.y -= 3
        }

        NumberAnimation on scale {
            running: true; from: 0.6; to: 1.0
            duration: Style.Theme.animBounce
            easing.type: Easing.OutBack
        }
    }
}
