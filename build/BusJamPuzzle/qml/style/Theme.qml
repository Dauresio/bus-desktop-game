/**
 * @file Theme.qml
 * @brief Singleton de design system para o Bus Jam Puzzle.
 *
 * Centraliza todas as decisões visuais: paleta de cores, tipografia,
 * raios de bordo, sombras e durações de animação.
 * Estilo cartoon: bordas grossas, cores vibrantes, sombras expressivas.
 *
 * Uso em QML:
 *   import "." as UI
 *   Rectangle { color: Theme.busColor(BusColor.Red) }
 */

pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    // ─── Paleta base ───────────────────────────────────────────────────────

    // Fundo do tabuleiro
    readonly property color bgPage:       "#1A1A2E"   // Azul-escuro profundo
    readonly property color bgBoard:      "#16213E"   // Painel do tabuleiro
    readonly property color bgCell:       "#0F3460"   // Célula vazia
    readonly property color bgCellAlt:    "#0D2E54"   // Célula alternada (xadrez)
    readonly property color bgOverlay:    "#CC000000"  // Overlay de pausa/vitória

    // HUD e UI
    readonly property color bgHUD:        "#E94560"   // Barra superior
    readonly property color bgButton:     "#F5A623"   // Botão principal
    readonly property color bgButtonHov:  "#E8941A"   // Hover/pressed
    readonly property color bgButtonDis:  "#888888"   // Desactivado

    // Texto
    readonly property color textPrimary:  "#FFFFFF"
    readonly property color textSecondary:"#BDC3C7"
    readonly property color textDark:     "#1A1A2E"
    readonly property color textScore:    "#F5A623"
    readonly property color textWin:      "#2ECC71"
    readonly property color textLose:     "#E74C3C"

    // Bordas cartoon (escuras e grossas)
    readonly property color borderDark:   "#1A1A2E"
    readonly property color borderLight:  "#FFFFFF"

    // Plataforma
    readonly property color platformRing: "#FFFFFF"
    readonly property color platformFree: "#1A1A2E"

    // Stars
    readonly property color starFill:     "#F5A623"
    readonly property color starEmpty:    "#555577"

    // ─── Cores dos autocarros / passageiros ────────────────────────────────
    // Cada cor tem: fill (corpo), stroke (bordo cartoon), light (janelas/brilho)

    readonly property var busColors: ({
        "Red":    { fill: "#E74C3C", stroke: "#922B21", light: "#F1948A" },
        "Blue":   { fill: "#3498DB", stroke: "#1A5276", light: "#85C1E9" },
        "Green":  { fill: "#2ECC71", stroke: "#1A7A44", light: "#82E0AA" },
        "Yellow": { fill: "#F1C40F", stroke: "#9A7D0A", light: "#F9E79F" },
        "Purple": { fill: "#9B59B6", stroke: "#6C3483", light: "#C39BD3" }
    })

    // Retorna a cor de preenchimento para um BusColor enum (int 0-4)
    function busColorFill(colorIndex) {
        var keys = ["Red","Blue","Green","Yellow","Purple"]
        return busColors[keys[colorIndex] || "Red"].fill
    }
    function busColorStroke(colorIndex) {
        var keys = ["Red","Blue","Green","Yellow","Purple"]
        return busColors[keys[colorIndex] || "Red"].stroke
    }
    function busColorLight(colorIndex) {
        var keys = ["Red","Blue","Green","Yellow","Purple"]
        return busColors[keys[colorIndex] || "Red"].light
    }
    // Versão por nome (string)
    function colorFillByName(name) {
        return (busColors[name] || busColors["Red"]).fill
    }
    function colorStrokeByName(name) {
        return (busColors[name] || busColors["Red"]).stroke
    }

    // ─── Bordas e raios ────────────────────────────────────────────────────

    readonly property int  borderWidth:   3    // Bordo cartoon padrão
    readonly property int  borderWidthHeavy: 4 // Bordo extra (bus, plataforma)
    readonly property int  radiusSmall:   6
    readonly property int  radiusMedium:  12
    readonly property int  radiusLarge:   20
    readonly property int  radiusBus:     10   // Autocarro
    readonly property int  radiusPassenger: 50 // Passageiro (círculo)

    // ─── Sombras cartoon ──────────────────────────────────────────────────
    // Implementadas em QML como Rectangle offset com opacidade

    readonly property int  shadowOffset:  4
    readonly property real shadowOpacity: 0.45
    readonly property color shadowColor:  "#1A1A2E"

    // ─── Tipografia ────────────────────────────────────────────────────────
    // Fonte principal: Fredoka One (cartoon, arredondada)
    // Fonte secundária: nunito (legível, amigável)

    readonly property string fontDisplay: "Fredoka One"
    readonly property string fontBody:    "Nunito"
    readonly property string fontMono:    "JetBrains Mono"

    readonly property int fontSizeXS:    10
    readonly property int fontSizeSM:    13
    readonly property int fontSizeMD:    16
    readonly property int fontSizeLG:    22
    readonly property int fontSizeXL:    32
    readonly property int fontSizeXXL:   48

    // ─── Espaçamentos ─────────────────────────────────────────────────────

    readonly property int spaceXS:  4
    readonly property int spaceSM:  8
    readonly property int spaceMD:  16
    readonly property int spaceLG:  24
    readonly property int spaceXL:  40

    // ─── Animações ────────────────────────────────────────────────────────

    readonly property int animFast:   150   // ms — feedback imediato
    readonly property int animMedium: 300   // ms — movimento de Bus
    readonly property int animSlow:   500   // ms — transições de ecrã
    readonly property int animBounce: 400   // ms — bounce de entidade
    readonly property string easingBounce: "Easing.OutBack"
    readonly property string easingSmooth: "Easing.InOutQuad"

    // ─── Dimensões de entidades (em unidades de célula) ───────────────────
    // Calculadas em Sizes.qml com base na resolução real

    // Proporção visual do autocarro em função da capacidade
    function busLengthFactor(capacity) {
        // 4 → 1 célula, 6 → 1.5, 8 → 2, 12 → 3
        return capacity / 4.0
    }

    // ─── Utilitários de cor ────────────────────────────────────────────────

    // Clareia uma cor hex em factor [0,1]
    function lighten(hexColor, factor) {
        return Qt.lighter(hexColor, 1.0 + factor)
    }

    // Escurece uma cor hex
    function darken(hexColor, factor) {
        return Qt.darker(hexColor, 1.0 + factor)
    }

    // Cor de texto legível sobre um fundo (branco ou escuro)
    function textOn(bgHex) {
        // Luminância aproximada
        var c = Qt.color(bgHex)
        var lum = 0.299*c.r + 0.587*c.g + 0.114*c.b
        return lum > 0.5 ? textDark : textPrimary
    }
}
