/**
 * @file PassengerItem.qml
 * @brief Componente visual de um passageiro no tabuleiro.
 *
 * Renderiza um passageiro como um círculo cartoon colorido com:
 * - Corpo preenchido com a cor da equipa
 * - Bordo cartoon escuro e grosso
 * - Brilho interior (highlight branco)
 * - Expressão simplificada (olhos)
 * - Animações de estado: idle bounce, boarding shrink, boarded invisible
 */

import QtQuick 2.15
import QtQuick.Effects
import "." as Components
import "../style" as Style

Item {
    id: root

    // ─── Propriedades públicas ─────────────────────────────────────────────

    property int   passengerId:   -1
    property int   colorIndex:    0      // BusColor enum (0-4)
    property bool  isBoarding:    false
    property bool  isBoarded:     false
    property real  cellSize:      Style.Sizes.cellSize

    // ─── Dimensões ─────────────────────────────────────────────────────────

    width:  cellSize
    height: cellSize

    // ─── Corpo principal ───────────────────────────────────────────────────

    // Sombra cartoon
    Rectangle {
        id: shadow
        width:  body.width
        height: body.height
        radius: width / 2
        color:  Style.Theme.shadowColor
        opacity: Style.Theme.shadowOpacity
        x: body.x + Style.Theme.shadowOffset
        y: body.y + Style.Theme.shadowOffset
        visible: !isBoarded
    }

    // Corpo
    Rectangle {
        id: body
        width:  Style.Sizes.passengerDiameter
        height: Style.Sizes.passengerDiameter
        radius: width / 2
        color:  Style.Theme.busColorFill(colorIndex)

        anchors.centerIn: parent

        border.color: Style.Theme.borderDark
        border.width: Style.Theme.borderWidth

        // Brilho interior (canto superior esquerdo)
        Rectangle {
            id: highlight
            width:  parent.width * 0.32
            height: parent.height * 0.32
            radius: width / 2
            color:  "#80FFFFFF"
            x: parent.width  * 0.18
            y: parent.height * 0.12
        }

        // Olho esquerdo
        Rectangle {
            width:  parent.width * 0.14
            height: parent.width * 0.14
            radius: width / 2
            color:  Style.Theme.borderDark
            x: parent.width * 0.28
            y: parent.height * 0.44
        }

        // Olho direito
        Rectangle {
            width:  parent.width * 0.14
            height: parent.width * 0.14
            radius: width / 2
            color:  Style.Theme.borderDark
            x: parent.width * 0.54
            y: parent.height * 0.44
        }

        // Sorriso (arco simulado com Rectangle rotacionado)
        Rectangle {
            width:  parent.width * 0.36
            height: parent.height * 0.12
            radius: height / 2
            color:  Style.Theme.borderDark
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.62
        }
    }

    // ─── Animações de estado ───────────────────────────────────────────────

    // Idle: bounce suave e contínuo
    SequentialAnimation on y {
        id: idleBounce
        running: !isBoarding && !isBoarded
        loops:   Animation.Infinite

        NumberAnimation {
            to:       -Style.Sizes.cellSize * 0.06
            duration: 900 + (passengerId % 3) * 150
            easing.type: Easing.InOutSine
        }
        NumberAnimation {
            to:       0
            duration: 900 + (passengerId % 3) * 150
            easing.type: Easing.InOutSine
        }
    }

    // Boarding: encolhe e desaparece
    SequentialAnimation {
        id: boardingAnim
        running: isBoarding

        ParallelAnimation {
            NumberAnimation {
                target:   body
                property: "scale"
                to:       0.0
                duration: Style.Theme.animMedium
                easing.type: Easing.InBack
            }
            NumberAnimation {
                target:   shadow
                property: "opacity"
                to:       0.0
                duration: Style.Theme.animMedium
            }
        }
    }

    // Visibilidade
    visible: !isBoarded
    opacity: isBoarding ? 1.0 : 1.0

    // ─── Transformação de escala global ───────────────────────────────────

    Behavior on scale {
        NumberAnimation {
            duration: Style.Theme.animFast
            easing.type: Easing.OutBack
        }
    }

    // ─── Estados ──────────────────────────────────────────────────────────

    states: [
        State {
            name: "boarding"
            when: isBoarding
            PropertyChanges { target: body; scale: 0.0 }
        },
        State {
            name: "boarded"
            when: isBoarded
            PropertyChanges { target: root; visible: false }
        }
    ]

    transitions: [
        Transition {
            from: ""
            to:   "boarding"
            NumberAnimation {
                target:   body
                property: "scale"
                duration: Style.Theme.animMedium
                easing.type: Easing.InBack
            }
        }
    ]
}
