/**
 * @file GameScreen.qml
 * @brief Ecrã principal de jogo do Bus Jam Puzzle.
 *
 * Compõe HUD + BoardView + overlays de pausa/vitória/derrota.
 * Liga-se ao GameController via propriedades e sinais.
 *
 * Layout responsivo:
 *  - Desktop: sidebar lateral direita + tabuleiro centrado
 *  - Mobile:  HUD topo + tabuleiro + barra inferior
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"
import "../style" as Style

Item {
    id: root

    // ─── Conexões ao GameController ────────────────────────────────────────

    Connections {
        target: gameController
        enabled: gameController !== null

        function onLevelWon(result) {
            boardView.showVictoryEffect = true
            winOverlay.stars  = gameController.starsForLevel(gameController.currentLevelId)
            winOverlay.score  = gameController.currentScore
            winOverlay.moves  = gameController.moveCount
            winOverlay.visible = true
        }

        function onLevelLost() {
            loseOverlay.visible = true
        }

        function onGameStateChanged() {
            pauseOverlay.visible =
                gameController.gameState === 3   // Paused
        }
    }

    // ─── Fundo ────────────────────────────────────────────────────────────

    Rectangle {
        anchors.fill: parent
        color: Style.Theme.bgPage
    }

    // Estrelas de fundo (decoração estática)
    Repeater {
        model: 30
        Rectangle {
            width:  1 + index % 3
            height: width
            radius: width
            color:  "#40FFFFFF"
            x: Math.random() * root.width
            y: Math.random() * root.height
            opacity: 0.3 + Math.random() * 0.5
        }
    }

    // ─── Layout principal ─────────────────────────────────────────────────

    // HUD topo
    HUD {
        id: hud
        anchors { top: parent.top; left: parent.left; right: parent.right }

        moveCount:          gameController ? gameController.moveCount          : 0
        elapsedSeconds:     gameController ? gameController.elapsedSeconds     : 0
        remainingSeconds:   gameController ? gameController.remainingSeconds   : 0
        currentScore:       gameController ? gameController.currentScore       : 0
        remainingPassengers: gameController ? gameController.remainingPassengers : 0
        hasTimeLimit:       gameController ? gameController.remainingSeconds > 0 : false
        canUndo:            gameController ? gameController.canUndo            : false
        canHint:            gameController ? gameController.canHint            : false

        onPauseRequested:   gameController && gameController.pauseGame()
        onRestartRequested: restartConfirmDialog.open()
        onUndoRequested:    gameController && gameController.requestUndo()
        onHintRequested:    gameController && gameController.requestHint()
    }

    // Área central: tabuleiro + sidebar (desktop)
    RowLayout {
        anchors {
            top:    hud.bottom
            bottom: parent.bottom
            left:   parent.left
            right:  parent.right
            margins: Style.Sizes.boardMargin
        }
        spacing: Style.Sizes.spaceMD

        // Tabuleiro centralizado
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            BoardView {
                id: boardView
                anchors.centerIn: parent

                boardRows:      (gameController && gameController.boardRows > 0) ? gameController.boardRows : 8
                boardCols:      (gameController && gameController.boardCols > 0) ? gameController.boardCols : 8
                busModel:       gameController ? gameController.busModel       : null
                passengerModel: gameController ? gameController.passengerModel : null
                platformModel:  gameController ? gameController.platformModel  : null

                onBusTapped: function(busId) {
                    if (gameController)
                        gameController.onBusTapped(busId)
                }
            }
        }

        // Sidebar (apenas desktop)
        Rectangle {
            visible: Style.Sizes.isDesktop
            Layout.preferredWidth: Style.Sizes.sidebarWidth
            Layout.fillHeight: true
            color:  Style.Theme.bgBoard
            radius: Style.Theme.radiusLarge
            border.color: Style.Theme.borderDark
            border.width: Style.Theme.borderWidth

            // Sombra
            Rectangle {
                width: parent.width; height: parent.height
                radius: parent.radius
                color: Style.Theme.shadowColor
                opacity: Style.Theme.shadowOpacity
                x: 4; y: 4; z: -1
            }

            Column {
                anchors {
                    fill: parent
                    margins: Style.Sizes.panelPadding
                }
                spacing: Style.Sizes.spaceMD

                // Título do nível
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: gameController ? "Nível " + gameController.currentLevelId : ""
                    color: Style.Theme.textPrimary
                    font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeLG }
                }

                // Separador
                Rectangle {
                    width: parent.width; height: 2
                    color: Style.Theme.borderDark
                    opacity: 0.3
                }

                // Passageiros restantes
                SidebarStat {
                    icon: "🚶"
                    label: "Passageiros"
                    value: gameController ? gameController.remainingPassengers : 0
                }

                // Autocarros restantes
                SidebarStat {
                    icon: "🚌"
                    label: "Autocarros"
                    value: gameController ? gameController.remainingBuses : 0
                }
            }
        }
    }

    // ─── Overlay de pausa ─────────────────────────────────────────────────

    Rectangle {
        id: pauseOverlay
        anchors.fill: parent
        color: Style.Theme.bgOverlay
        visible: false
        z: 10

        Column {
            anchors.centerIn: parent
            spacing: Style.Sizes.spaceLG

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "⏸ PAUSA"
                color: Style.Theme.textPrimary
                font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeXL }
            }

            OverlayButton {
                text: "▶  Continuar"
                onClicked: gameController && gameController.resumeGame()
            }
            OverlayButton {
                text: "↺  Recomeçar"
                onClicked: { pauseOverlay.visible = false; gameController && gameController.restartLevel() }
            }
            OverlayButton {
                text: "🏠 Menu"
                onClicked: gameController && gameController.goToMenu()
                bgColor: "#555577"
            }
        }
    }

    // ─── Overlay de vitória ───────────────────────────────────────────────

    Rectangle {
        id: winOverlay
        anchors.fill: parent
        color: Style.Theme.bgOverlay
        visible: false
        z: 10

        property int stars: 0
        property int score: 0
        property int moves: 0

        Column {
            anchors.centerIn: parent
            spacing: Style.Sizes.spaceLG

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "🎉 PARABÉNS!"
                color: Style.Theme.textWin
                font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeXL }

                NumberAnimation on scale {
                    running: winOverlay.visible
                    from: 0.5; to: 1.0
                    duration: Style.Theme.animBounce
                    easing.type: Easing.OutBack
                }
            }

            // Estrelas
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: Style.Sizes.spaceSM

                Repeater {
                    model: 3
                    Text {
                        text: "★"
                        color: index < winOverlay.stars
                               ? Style.Theme.starFill
                               : Style.Theme.starEmpty
                        font.pixelSize: Style.Sizes.fontSizeXL

                        SequentialAnimation {
                            running: winOverlay.visible && index < winOverlay.stars
                            PauseAnimation { duration: index * 150 }
                            NumberAnimation {
                                target: parent; property: "scale"
                                from: 0.0; to: 1.0
                                duration: Style.Theme.animBounce
                                easing.type: Easing.OutBack
                            }
                        }
                    }
                }
            }

            // Score
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Score: " + winOverlay.score
                color: Style.Theme.textScore
                font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeLG }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Jogadas: " + winOverlay.moves
                color: Style.Theme.textSecondary
                font { family: Style.Theme.fontBody; pixelSize: Style.Sizes.fontSizeMD }
            }

            OverlayButton {
                text: "▶▶ Próximo Nível"
                onClicked: { winOverlay.visible = false; gameController && gameController.nextLevel() }
                bgColor: Style.Theme.textWin
            }
            OverlayButton {
                text: "↺  Repetir"
                onClicked: { winOverlay.visible = false; gameController && gameController.restartLevel() }
            }
            OverlayButton {
                text: "🏠 Menu"
                onClicked: gameController && gameController.goToMenu()
                bgColor: "#555577"
            }
        }
    }

    // ─── Overlay de derrota ───────────────────────────────────────────────

    Rectangle {
        id: loseOverlay
        anchors.fill: parent
        color: Style.Theme.bgOverlay
        visible: false
        z: 10

        Column {
            anchors.centerIn: parent
            spacing: Style.Sizes.spaceLG

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "⏰ TEMPO ESGOTADO"
                color: Style.Theme.textLose
                font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeXL }
            }

            OverlayButton {
                text: "↺  Tentar Novamente"
                onClicked: { loseOverlay.visible = false; gameController && gameController.restartLevel() }
            }
            OverlayButton {
                text: "🏠 Menu"
                onClicked: gameController && gameController.goToMenu()
                bgColor: "#555577"
            }
        }
    }

    // ─── Dialog de confirmação de restart ─────────────────────────────────

    Dialog {
        id: restartConfirmDialog
        width: 460
        x: Math.round((root.width  - width)  / 2)
        y: Math.round((root.height - height) / 2)
        title: "Recomeçar nível?"
        modal: true

        background: Rectangle {
            color: Style.Theme.bgBoard
            radius: Style.Theme.radiusLarge
            border.color: Style.Theme.borderDark
            border.width: Style.Theme.borderWidthHeavy
        }

        contentItem: Column {
            spacing: Style.Sizes.spaceMD
            padding: Style.Sizes.spaceMD

            Text {
                text: "O progresso deste nível será perdido."
                color: Style.Theme.textSecondary
                font { family: Style.Theme.fontBody; pixelSize: Style.Sizes.fontSizeMD }
            }

            Row {
                spacing: Style.Sizes.spaceSM

                OverlayButton {
                    text: "Cancelar"
                    bgColor: "#555577"
                    onClicked: restartConfirmDialog.close()
                }
                OverlayButton {
                    text: "↺ Recomeçar"
                    onClicked: {
                        restartConfirmDialog.close()
                        gameController && gameController.restartLevel()
                    }
                }
            }
        }
    }

    // ─── Componentes internos reutilizáveis ───────────────────────────────

    component OverlayButton: Rectangle {
        property string text:    ""
        property color  bgColor: Style.Theme.bgButton
        signal clicked()

        width:  Math.max(200, Style.Sizes.sidebarWidth)
        height: Style.Sizes.buttonHeight
        radius: Style.Sizes.buttonRadius
        color:  bgColor
        border.color: Style.Theme.borderDark
        border.width: Style.Theme.borderWidth

        Rectangle {
            width: parent.width; height: parent.height
            radius: parent.radius
            color: Style.Theme.shadowColor
            opacity: Style.Theme.shadowOpacity
            x: 3; y: 3; z: -1
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
        }

        scale: containsMouse ? 1.03 : 1.0
        property bool containsMouse: false

        HoverHandler { onHoveredChanged: parent.containsMouse = hovered }

        Behavior on scale { NumberAnimation { duration: 100 } }
    }

    component SidebarStat: Row {
        property string icon:  ""
        property string label: ""
        property int    value: 0
        spacing: Style.Sizes.spaceSM

        Text { text: parent.icon; font.pixelSize: Style.Sizes.fontSizeMD }
        Column {
            Text {
                text: parent.parent.label
                color: Style.Theme.textSecondary
                font { family: Style.Theme.fontBody; pixelSize: Style.Sizes.fontSizeSM }
            }
            Text {
                text: parent.parent.value.toString()
                color: Style.Theme.textPrimary
                font { family: Style.Theme.fontDisplay; pixelSize: Style.Sizes.fontSizeLG }
            }
        }
    }
}
