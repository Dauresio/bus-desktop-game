/**
 * @file main.qml
 * @brief Raiz da aplicação Bus Jam Puzzle.
 *
 * Gere a navegação entre ecrãs via StackView e liga o GameController
 * a todos os ecrãs. Responsável por:
 * - Janela principal (Window)
 * - StackView de navegação
 * - Conexões globais ao GameController
 * - Carregamento de fontes
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import "screens"
import "style" as Style

Window {
    id: appWindow

    // ─── Dimensões responsivas ────────────────────────────────────────────

    minimumWidth:  360
    minimumHeight: 600
    width:  Style.Sizes.isDesktop ? 900 : 390
    height: Style.Sizes.isDesktop ? 680 : 844

    title: "Bus Jam Puzzle"
    visible: true

    // Actualiza Sizes quando a janela redimensiona
    onWidthChanged:  { Style.Sizes.availableWidth  = width  }
    onHeightChanged: { Style.Sizes.availableHeight = height }

    Component.onCompleted: {
        Style.Sizes.availableWidth  = width
        Style.Sizes.availableHeight = height
    }

    // ─── Fundo global ─────────────────────────────────────────────────────

    color: Style.Theme.bgPage

    // ─── Navegação principal ──────────────────────────────────────────────

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: mainMenuComponent

        // Transição de ecrã cartoon: slide + fade
        pushEnter: Transition {
            ParallelAnimation {
                NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Style.Theme.animSlow }
                NumberAnimation { property: "x"; from: appWindow.width; to: 0; duration: Style.Theme.animSlow; easing.type: Easing.OutCubic }
            }
        }
        pushExit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Style.Theme.animSlow }
        }
        popEnter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Style.Theme.animSlow }
        }
        popExit: Transition {
            ParallelAnimation {
                NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Style.Theme.animSlow }
                NumberAnimation { property: "x"; from: 0; to: appWindow.width; duration: Style.Theme.animSlow; easing.type: Easing.InCubic }
            }
        }
    }

    // ─── Componentes de ecrã ─────────────────────────────────────────────

    Component {
        id: mainMenuComponent
        MainMenuScreen {
            width: appWindow.width; height: appWindow.height

            onPlayRequested: {
                stackView.push(levelSelectComponent)
            }
            onSettingsRequested: {
                // Futuro: stackView.push(settingsComponent)
            }
            onQuitRequested: {
                Qt.quit()
            }
        }
    }

    Component {
        id: levelSelectComponent
        LevelSelectScreen {
            width: appWindow.width; height: appWindow.height
            highestUnlocked: gameController ? gameController.highestUnlockedLevel : 1
            levelMetaList:   buildLevelMetaList()

            onLevelSelected: function(levelId) {
                if (gameController)
                    gameController.loadLevel(levelId)
                stackView.push(gameScreenComponent)
            }
            onBackRequested: stackView.pop()
        }
    }

    Component {
        id: gameScreenComponent
        GameScreen {
            width: appWindow.width; height: appWindow.height

            // Constrói os modelos de dados ao entrar no ecrã
            Component.onCompleted: {
                buildBoardModels()
            }

            // Actualiza modelos quando o nível é (re)carregado
            Connections {
                target: gameController
                function onCurrentLevelChanged() {
                    buildBoardModels()
                }
            }
        }
    }

    // ─── Conexões globais ao GameController ───────────────────────────────

    Connections {
        target: gameController
        enabled: gameController !== null

        function onGameStateChanged() {
            const state = gameController.gameState

            // Menu (0)
            if (state === 0) {
                stackView.clear()
                stackView.push(mainMenuComponent)
            }
            // LevelSelection (1)
            else if (state === 1) {
                if (stackView.currentItem instanceof LevelSelectScreen)
                    return
                stackView.push(levelSelectComponent)
            }
        }

        function onErrorOccurred(message) {
            errorToast.show(message)
        }
    }

    // ─── Toast de erro ────────────────────────────────────────────────────

    Rectangle {
        id: errorToast
        anchors { bottom: parent.bottom; horizontalCenter: parent.horizontalCenter; bottomMargin: 40 }
        width: Math.min(parent.width - 40, 360)
        height: 52
        radius: Style.Theme.radiusMedium
        color: Style.Theme.textLose
        border.color: Style.Theme.borderDark; border.width: Style.Theme.borderWidth
        visible: false
        z: 100

        property string message: ""
        function show(msg) {
            message = msg
            visible = true
            hideTimer.restart()
        }

        Text {
            anchors.centerIn: parent
            text: errorToast.message
            color: Style.Theme.textPrimary
            font { family: Style.Theme.fontBody; pixelSize: Style.Sizes.fontSizeMD }
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: parent.width - 24
        }

        Timer { id: hideTimer; interval: 3000; onTriggered: errorToast.visible = false }

        NumberAnimation on opacity {
            running: !errorToast.visible
            from: 1; to: 0; duration: 300
        }
    }

    // ─── Funções auxiliares ───────────────────────────────────────────────

    function buildLevelMetaList() {
        // Quando LevelManager estiver integrado via context property,
        // esta função devolve a lista real de metadados.
        // Por agora, gera níveis de placeholder.
        if (typeof levelManager !== "undefined" && levelManager !== null) {
            return levelManager.allLevelMetadata()
        }
        // Placeholder: 10 níveis
        var list = []
        for (var i = 1; i <= 10; ++i) {
            list.push({
                id: i,
                name: "Nível " + i,
                difficulty: Math.min(5, Math.ceil(i / 4)),
                stars: gameController ? gameController.starsForLevel(i) : 0,
                unlocked: i <= (gameController ? gameController.highestUnlockedLevel : 1)
            })
        }
        return list
    }

    function buildBoardModels() {
        // Em produção, o GameController expõe propriedades de lista
        // (busListModel, passengerListModel, platformListModel).
        // Esta função constrói os arrays QML a partir dessas propriedades.
        //
        // Placeholder — a integração real depende de QAbstractListModel
        // ou de propriedades Q_PROPERTY expostas pelo GameController.
        console.log("buildBoardModels: integração com GameController pendente.")
    }
}
