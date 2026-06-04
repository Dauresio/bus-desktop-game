/**
 * @file LevelSelectScreen.qml
 * @brief Ecrã de seleção de nível — grelha de níveis com estado de progresso.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../style" as Style

Item {
    id: root

    property var    levelMetaList:  []    // Array de { id, name, difficulty, stars, unlocked }
    property int    highestUnlocked: 1

    signal levelSelected(int levelId)
    signal backRequested()

    // ─── Fundo ────────────────────────────────────────────────────────────

    Rectangle { anchors.fill: parent; color: Style.Theme.bgPage }

    // ─── Cabeçalho ────────────────────────────────────────────────────────

    Rectangle {
        id: header
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: Style.Sizes.hudHeight
        color:  Style.Theme.bgHUD
        border.color: Style.Theme.borderDark; border.width: Style.Theme.borderWidthHeavy

        RowLayout {
            anchors { fill: parent; margins: Style.Sizes.spaceMD }

            // Botão voltar
            Rectangle {
                width: Style.Sizes.buttonHeight; height: Style.Sizes.buttonHeight
                radius: Style.Sizes.buttonRadius
                color: Style.Theme.bgButton
                border.color: Style.Theme.borderDark; border.width: Style.Theme.borderWidth

                Rectangle {
                    width: parent.width; height: parent.height; radius: parent.radius
                    color: Style.Theme.shadowColor; opacity: Style.Theme.shadowOpacity
                    x: 3; y: 3; z: -1
                }

                Text {
                    anchors.centerIn: parent; text: "←"
                    font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeLG }
                    color: Style.Theme.textPrimary
                }
                MouseArea { anchors.fill: parent; onClicked: root.backRequested() }
            }

            Text {
                Layout.fillWidth: true
                text: "Escolher Nível"
                horizontalAlignment: Text.AlignHCenter
                color: Style.Theme.textPrimary
                font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeLG }
            }

            Item { width: Style.Sizes.buttonHeight; height: Style.Sizes.buttonHeight }
        }
    }

    // ─── Grelha de níveis ─────────────────────────────────────────────────

    ScrollView {
        anchors {
            top: header.bottom; bottom: parent.bottom
            left: parent.left; right: parent.right
            margins: Style.Sizes.spaceMD
        }
        clip: true

        GridView {
            id: levelGrid
            anchors.fill: parent
            cellWidth:  Style.Sizes.isMobile ? root.width / 3 : 120
            cellHeight: cellWidth * 1.15
            model:      root.levelMetaList

            delegate: Item {
                width:  levelGrid.cellWidth
                height: levelGrid.cellHeight

                readonly property bool isUnlocked: modelData.id <= root.highestUnlocked

                // Card do nível
                Rectangle {
                    anchors { fill: parent; margins: 6 }
                    radius: Style.Theme.radiusMedium
                    color:  isUnlocked ? Style.Theme.bgBoard : "#22333355"
                    border.color: isUnlocked
                                  ? Style.Theme.busColorFill(modelData.id % 5)
                                  : Style.Theme.borderDark
                    border.width: Style.Theme.borderWidthHeavy
                    opacity: isUnlocked ? 1.0 : 0.5

                    // Sombra
                    Rectangle {
                        width: parent.width; height: parent.height
                        radius: parent.radius
                        color: Style.Theme.shadowColor; opacity: isUnlocked ? Style.Theme.shadowOpacity : 0
                        x: 3; y: 3; z: -1
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 4

                        // Número do nível
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: isUnlocked ? modelData.id.toString() : "🔒"
                            font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeXL }
                            color: isUnlocked
                                   ? Style.Theme.busColorFill(modelData.id % 5)
                                   : Style.Theme.textSecondary
                        }

                        // Estrelas
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 2
                            Repeater {
                                model: 3
                                Text {
                                    text: "★"
                                    font.pixelSize: Style.Sizes.fontSizeSM
                                    color: (isUnlocked && index < (modelData.stars || 0))
                                           ? Style.Theme.starFill
                                           : Style.Theme.starEmpty
                                }
                            }
                        }

                        // Dificuldade (pontinhos)
                        Row {
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 3
                            visible: isUnlocked
                            Repeater {
                                model: modelData.difficulty || 1
                                Rectangle {
                                    width: 5; height: 5; radius: 3
                                    color: Style.Theme.bgHUD
                                }
                            }
                        }
                    }

                    // Área de toque
                    MouseArea {
                        anchors.fill: parent
                        enabled: isUnlocked
                        onClicked: root.levelSelected(modelData.id)
                        onPressed: parent.scale = 0.93
                        onReleased: parent.scale = 1.0
                    }

                    Behavior on scale { NumberAnimation { duration: 80 } }
                }
            }
        }
    }
}
