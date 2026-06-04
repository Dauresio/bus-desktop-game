/**
 * @file Sizes.qml
 * @brief Singleton de dimensões responsivas para o Bus Jam Puzzle.
 *
 * Calcula todas as dimensões derivadas do tamanho do ecrã e do número
 * de colunas/linhas do tabuleiro atual. Actualizado dinamicamente quando
 * a janela redimensiona ou o nível muda.
 *
 * Convenção: "cell" = uma célula do tabuleiro.
 * Todos os componentes usam cellSize para se posicionar e dimensionar.
 */

pragma Singleton
import QtQuick 2.15
import QtQuick.Window 2.15

QtObject {
    id: root

    // ─── Parâmetros de entrada (actualizados pelo GameController) ──────────

    property int boardRows: 8
    property int boardCols: 8

    // ─── Detecção de plataforma ────────────────────────────────────────────

    readonly property bool isMobile: Screen.pixelDensity > 5.0
                                     || Screen.width < 600
    readonly property bool isTablet: !isMobile && Screen.width < 1024
    readonly property bool isDesktop: !isMobile && !isTablet

    // ─── Dimensões da janela disponível ───────────────────────────────────

    // Área útil para o tabuleiro (descontando HUD e margens)
    property real availableWidth:  Screen.width
    property real availableHeight: Screen.height

    readonly property real hudHeight:     isMobile ? 56 : 64
    readonly property real sidebarWidth:  isMobile ? 0  : 180
    readonly property real boardMargin:   isMobile ? 8  : 20

    readonly property real boardAreaWidth:  availableWidth  - sidebarWidth - boardMargin * 2
    readonly property real boardAreaHeight: availableHeight - hudHeight    - boardMargin * 2

    // ─── Tamanho de célula ─────────────────────────────────────────────────

    // Calculado para caber o tabuleiro inteiro na área disponível.
    // Usa o mínimo entre largura/colunas e altura/linhas.
    readonly property real cellSize: Math.min(
        Math.floor(boardAreaWidth  / boardCols),
        Math.floor(boardAreaHeight / boardRows)
    )

    // Tamanho total do tabuleiro renderizado
    readonly property real boardPixelWidth:  cellSize * boardCols
    readonly property real boardPixelHeight: cellSize * boardRows

    // ─── Dimensões de entidades ────────────────────────────────────────────

    // Passageiro: círculo centrado na célula, com padding
    readonly property real passengerDiameter: cellSize * 0.68
    readonly property real passengerPadding:  (cellSize - passengerDiameter) / 2

    // Bus: altura = célula, largura proporcional à capacidade
    readonly property real busHeight: cellSize * 0.82
    readonly property real busVertPadding: (cellSize - busHeight) / 2

    // Plataforma: quadrado com anel de cor
    readonly property real platformSize:      cellSize * 0.9
    readonly property real platformRingWidth: Math.max(3, cellSize * 0.08)

    // ─── Tipografia escalada ───────────────────────────────────────────────

    readonly property real fontScaleFactor: isMobile ? 0.85 : 1.0

    readonly property real fontSizeSM:  Math.round(13 * fontScaleFactor)
    readonly property real fontSizeMD:  Math.round(16 * fontScaleFactor)
    readonly property real fontSizeLG:  Math.round(22 * fontScaleFactor)
    readonly property real fontSizeXL:  Math.round(32 * fontScaleFactor)
    readonly property real fontSizeXXL: Math.round(isMobile ? 36 : 48)

    // ─── Touch targets ────────────────────────────────────────────────────

    // Em mobile, os touch targets nunca devem ser menores que 44px (HIG).
    readonly property real minTouchTarget: 44
    readonly property real buttonHeight: isMobile ? 52 : 44
    readonly property real buttonRadius: isMobile ? 16 : 12

    // ─── Espaçamento ──────────────────────────────────────────────────────

    readonly property real spaceXS: isMobile ? 4  : 6
    readonly property real spaceSM: isMobile ? 8  : 10
    readonly property real spaceMD: isMobile ? 12 : 16
    readonly property real spaceLG: isMobile ? 18 : 24

    // ─── Sidebar e painéis ────────────────────────────────────────────────

    readonly property real panelPadding: isMobile ? 12 : 20
    readonly property real iconSize:     isMobile ? 28 : 24

    // ─── Animações escaladas ──────────────────────────────────────────────
    // Em dispositivos lentos, reduz duração automaticamente.
    // (Controlado por setting futuro de "reduzir animações")

    property bool reduceMotion: false

    readonly property int animMultiplier: reduceMotion ? 0 : 1

    function anim(baseDuration) {
        return baseDuration * animMultiplier
    }
}
