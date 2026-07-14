// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as LP

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D
import org.deepin.ds.dock 1.0

AppletItem {
    id: root

    property bool useColumnLayout: Panel.position % 2
    property int dockOrder: 17
    property bool shouldVisible: true

    implicitWidth: useColumnLayout ? Panel.rootObject.dockSize : Panel.rootObject.dockItemMaxSize * 0.8
    implicitHeight: useColumnLayout ? Panel.rootObject.dockItemMaxSize * 0.8 : Panel.rootObject.dockSize

    // --- Hover / Press background (aligned to canvas + 6px margin) ---
    property real canvasSize: Math.min(root.width, root.height) * 0.75
    property real hoverSize: canvasSize * 1.4

    Rectangle {
        id: hoverBg
        anchors.centerIn: parent
        width: hoverSize
        height: hoverSize
        radius: 12
        color: "transparent"

        Behavior on color { ColorAnimation { duration: 120 } }

        states: State {
            when: mouseHandler.containsMouse
            PropertyChanges {
                hoverBg.color: Qt.rgba(1.0, 1.0, 1.0, 0.15)
            }
        }
    }

    Rectangle {
        id: pressBg
        anchors.centerIn: parent
        width: hoverSize
        height: hoverSize
        radius: 12
        color: "transparent"

        Behavior on color { ColorAnimation { duration: 80 } }

        states: State {
            when: mouseHandler.pressed
            PropertyChanges {
                pressBg.color: Qt.rgba(1.0, 1.0, 1.0, 0.25)
            }
        }
    }

    // --- Tooltip ---
    PanelToolTip {
        id: toolTip
        text: qsTr("Files: %1").arg(Applet.folderCount)
        toolTipX: DockPanelPositioner.x
        toolTipY: DockPanelPositioner.y
    }

    // --- Dock item canvas ---
    Canvas {
        id: iconCanvas
        anchors.centerIn: parent
        width: Math.min(root.width, root.height) * 0.75
        height: width
        renderTarget: Canvas.FramebufferObject

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Component.onCompleted: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            ctx.clearRect(0, 0, width, height);

            var theme = getTheme(Applet.colorTheme);

            drawGridFolders(ctx, width, height, Applet.displayFolderCount, theme);

            // Draw count badge when total entries > 4
            if (Applet.displayFolderCount >= 4 && Applet.directoryModel) {
                var total = Applet.directoryModel.totalCount;
                if (total > 4) {
                    drawCountBadge(ctx, width, height, total, theme);
                }
            }
        }

        // --- Color theme definitions ---
        function getTheme(idx) {
            var themes = [
                { colors: ["#0096C7", "#00B4D8", "#48CAE4", "#90E0EF"],
                  border: "rgba(0,180,216,0.5)" },
                { colors: ["#E63946", "#F4845F", "#F7B267", "#F4D35E"],
                  border: "rgba(244,132,95,0.5)" },
                { colors: ["#06D6A0", "#7B2FF7", "#B388FF", "#F15BB5"],
                  border: "rgba(123,47,247,0.5)" },
                { colors: ["#39FF14", "#00FFFF", "#0080FF", "#FF00FF"],
                  border: "rgba(57,255,20,0.5)" },
            ];
            return themes[idx] || themes[0];
        }

        function darken(hex, f) {
            var r = parseInt(hex.slice(1, 3), 16);
            var g = parseInt(hex.slice(3, 5), 16);
            var b = parseInt(hex.slice(5, 7), 16);
            r = Math.round(r * f); g = Math.round(g * f); b = Math.round(b * f);
            return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1);
        }

        function hexToRgba(hex, a) {
            var r = parseInt(hex.slice(1, 3), 16);
            var g = parseInt(hex.slice(3, 5), 16);
            var b = parseInt(hex.slice(5, 7), 16);
            return "rgba(" + r + "," + g + "," + b + "," + a + ")";
        }

        function roundedRect(ctx, x, y, w, h, r, fill, stroke, lineW) {
            ctx.beginPath();
            ctx.moveTo(x + r, y);
            ctx.lineTo(x + w - r, y);
            ctx.quadraticCurveTo(x + w, y, x + w, y + r);
            ctx.lineTo(x + w, y + h - r);
            ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
            ctx.lineTo(x + r, y + h);
            ctx.quadraticCurveTo(x, y + h, x, y + h - r);
            ctx.lineTo(x, y + r);
            ctx.quadraticCurveTo(x, y, x + r, y);
            ctx.closePath();
            if (fill) { ctx.fillStyle = fill; ctx.fill(); }
            if (stroke) { ctx.strokeStyle = stroke; ctx.lineWidth = lineW || 1; ctx.stroke(); }
        }

        // --- Single folder icon ---
        function drawFolderIcon(ctx, x, y, w, h, color1, color2) {
            var r = Math.max(1, Math.min(w, h) * 0.08);
            var tabH = h * 0.2;
            var tabW = w * 0.4;
            var tabR = Math.max(1, r * 0.8);

            ctx.beginPath();
            ctx.moveTo(x + tabR, y);
            ctx.lineTo(x + tabW - tabR, y);
            ctx.quadraticCurveTo(x + tabW, y, x + tabW, y + tabR);
            ctx.lineTo(x + tabW + tabH * 0.4, y + tabH);
            ctx.lineTo(x, y + tabH);
            ctx.lineTo(x, y + tabR);
            ctx.quadraticCurveTo(x, y, x + tabR, y);
            ctx.closePath();
            ctx.fillStyle = color1;
            ctx.fill();
            ctx.strokeStyle = "rgba(255,255,255,0.25)";
            ctx.lineWidth = 0.5;
            ctx.stroke();

            ctx.beginPath();
            ctx.moveTo(x + r, y + tabH);
            ctx.lineTo(x + w - r, y + tabH);
            ctx.quadraticCurveTo(x + w, y + tabH, x + w, y + tabH + r);
            ctx.lineTo(x + w, y + h - r);
            ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
            ctx.lineTo(x + r, y + h);
            ctx.quadraticCurveTo(x, y + h, x, y + h - r);
            ctx.lineTo(x, y + tabH + r);
            ctx.quadraticCurveTo(x, y + tabH, x + r, y + tabH);
            ctx.closePath();

            var grad = ctx.createLinearGradient(x, y + tabH, x, y + h);
            grad.addColorStop(0, color1);
            grad.addColorStop(1, color2);
            ctx.fillStyle = grad;
            ctx.fill();
            ctx.strokeStyle = "rgba(255,255,255,0.2)";
            ctx.lineWidth = 0.5;
            ctx.stroke();
        }

        // --- File icon (generic document with folded corner) ---
        function drawFileIcon(ctx, x, y, w, h, color1, color2) {
            var r = Math.max(1, Math.min(w, h) * 0.06);
            var fold = w * 0.25;

            // Main body
            ctx.beginPath();
            ctx.moveTo(x + r, y);
            ctx.lineTo(x + w - fold, y);
            ctx.lineTo(x + w, y + fold);
            ctx.lineTo(x + w, y + h - r);
            ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
            ctx.lineTo(x + r, y + h);
            ctx.quadraticCurveTo(x, y + h, x, y + h - r);
            ctx.lineTo(x, y + r);
            ctx.quadraticCurveTo(x, y, x + r, y);
            ctx.closePath();

            var grad = ctx.createLinearGradient(x, y, x, y + h);
            grad.addColorStop(0, color1);
            grad.addColorStop(1, color2);
            ctx.fillStyle = grad;
            ctx.fill();
            ctx.strokeStyle = "rgba(255,255,255,0.2)";
            ctx.lineWidth = 0.5;
            ctx.stroke();

            // Fold triangle
            ctx.beginPath();
            ctx.moveTo(x + w - fold, y);
            ctx.lineTo(x + w - fold, y + fold);
            ctx.lineTo(x + w, y + fold);
            ctx.closePath();
            ctx.fillStyle = darken(color1, 0.7);
            ctx.fill();
            ctx.strokeStyle = "rgba(255,255,255,0.15)";
            ctx.lineWidth = 0.5;
            ctx.stroke();

            // Text lines
            var lineY = y + h * 0.45;
            var lineH = h * 0.05;
            var lineGap = h * 0.09;
            for (var i = 0; i < 3; i++) {
                var lw = (i === 2) ? w * 0.4 : w * 0.55;
                ctx.fillStyle = "rgba(255,255,255,0.3)";
                roundedRect(ctx, x + w * 0.18, lineY + i * lineGap, lw, lineH, lineH / 2, "rgba(255,255,255,0.3)", null);
            }
        }

        // --- Image file icon (landscape with sun) ---
        function drawImageIcon(ctx, x, y, w, h, color1, color2) {
            var r = Math.max(1, Math.min(w, h) * 0.08);
            roundedRect(ctx, x, y, w, h, r, null, "rgba(255,255,255,0.2)", 0.5);

            var grad = ctx.createLinearGradient(x, y, x, y + h);
            grad.addColorStop(0, color1);
            grad.addColorStop(1, color2);
            ctx.fillStyle = grad;
            roundedRect(ctx, x, y, w, h, r, grad, "rgba(255,255,255,0.2)", 0.5);

            // Mountain triangle
            ctx.beginPath();
            ctx.moveTo(x + w * 0.15, y + h * 0.75);
            ctx.lineTo(x + w * 0.45, y + h * 0.35);
            ctx.lineTo(x + w * 0.65, y + h * 0.75);
            ctx.closePath();
            ctx.fillStyle = "rgba(255,255,255,0.25)";
            ctx.fill();

            // Second mountain
            ctx.beginPath();
            ctx.moveTo(x + w * 0.45, y + h * 0.75);
            ctx.lineTo(x + w * 0.7, y + h * 0.45);
            ctx.lineTo(x + w * 0.88, y + h * 0.75);
            ctx.closePath();
            ctx.fillStyle = "rgba(255,255,255,0.18)";
            ctx.fill();

            // Sun
            ctx.beginPath();
            ctx.arc(x + w * 0.72, y + h * 0.28, w * 0.1, 0, Math.PI * 2);
            ctx.fillStyle = "rgba(255,255,255,0.4)";
            ctx.fill();
        }

        // --- Video file icon (film strip with play button) ---
        function drawVideoIcon(ctx, x, y, w, h, color1, color2) {
            var r = Math.max(1, Math.min(w, h) * 0.06);
            var grad = ctx.createLinearGradient(x, y, x, y + h);
            grad.addColorStop(0, color1);
            grad.addColorStop(1, color2);
            roundedRect(ctx, x, y, w, h, r, grad, "rgba(255,255,255,0.2)", 0.5);

            // Film strip holes on left
            var holeW = w * 0.08;
            var holeH = h * 0.12;
            var holeX = x + w * 0.06;
            for (var i = 0; i < 3; i++) {
                roundedRect(ctx, holeX, y + h * 0.15 + i * h * 0.25, holeW, holeH, 1, "rgba(0,0,0,0.2)", null);
            }

            // Film strip holes on right
            var holeX2 = x + w - w * 0.06 - holeW;
            for (var i = 0; i < 3; i++) {
                roundedRect(ctx, holeX2, y + h * 0.15 + i * h * 0.25, holeW, holeH, 1, "rgba(0,0,0,0.2)", null);
            }

            // Play button triangle
            var playCx = x + w * 0.5;
            var playCy = y + h * 0.5;
            var playS = w * 0.2;
            ctx.beginPath();
            ctx.moveTo(playCx - playS * 0.4, playCy - playS * 0.6);
            ctx.lineTo(playCx + playS * 0.6, playCy);
            ctx.lineTo(playCx - playS * 0.4, playCy + playS * 0.6);
            ctx.closePath();
            ctx.fillStyle = "rgba(255,255,255,0.45)";
            ctx.fill();
        }

        // --- Audio file icon (music note) ---
        function drawAudioIcon(ctx, x, y, w, h, color1, color2) {
            var r = Math.max(1, Math.min(w, h) * 0.06);
            var grad = ctx.createLinearGradient(x, y, x, y + h);
            grad.addColorStop(0, color1);
            grad.addColorStop(1, color2);
            roundedRect(ctx, x, y, w, h, r, grad, "rgba(255,255,255,0.2)", 0.5);

            // Music note
            var noteX = x + w * 0.35;
            var noteTopY = y + h * 0.22;
            var noteH = h * 0.4;
            var noteW = w * 0.06;

            // Stem
            ctx.fillStyle = "rgba(255,255,255,0.45)";
            ctx.fillRect(noteX, noteTopY, noteW, noteH);

            // Note head (ellipse)
            ctx.beginPath();
            ctx.ellipse(noteX - w * 0.04, noteTopY + noteH, w * 0.14, h * 0.1, -0.3, 0, Math.PI * 2);
            ctx.fillStyle = "rgba(255,255,255,0.45)";
            ctx.fill();

            // Second note
            var note2X = x + w * 0.6;
            ctx.fillRect(note2X, noteTopY + h * 0.05, noteW, noteH);
            ctx.beginPath();
            ctx.ellipse(note2X - w * 0.04, noteTopY + h * 0.05 + noteH, w * 0.14, h * 0.1, -0.3, 0, Math.PI * 2);
            ctx.fill();

            // Beam connecting stems
            ctx.fillStyle = "rgba(255,255,255,0.45)";
            ctx.beginPath();
            ctx.moveTo(noteX, noteTopY);
            ctx.lineTo(note2X, noteTopY + h * 0.05);
            ctx.lineTo(note2X + noteW, noteTopY + h * 0.05);
            ctx.lineTo(noteX + noteW, noteTopY);
            ctx.closePath();
            ctx.fill();
        }

        // --- Script file icon (terminal with ">_" prompt) ---
        function drawScriptIcon(ctx, x, y, w, h, color1, color2) {
            var r = Math.max(1, Math.min(w, h) * 0.06);
            var grad = ctx.createLinearGradient(x, y, x, y + h);
            grad.addColorStop(0, color1);
            grad.addColorStop(1, color2);
            roundedRect(ctx, x, y, w, h, r, grad, "rgba(255,255,255,0.2)", 0.5);

            // Title bar
            var barH = h * 0.14;
            roundedRect(ctx, x + 1, y + 1, w - 2, barH, r, "rgba(0,0,0,0.15)", null);

            // Title bar dots
            var dotR = barH * 0.18;
            var dotY = y + barH * 0.5;
            for (var i = 0; i < 3; i++) {
                ctx.beginPath();
                ctx.arc(x + w * 0.15 + i * w * 0.1, dotY, dotR, 0, Math.PI * 2);
                ctx.fillStyle = "rgba(255,255,255," + (0.5 - i * 0.1) + ")";
                ctx.fill();
            }

            // Prompt ">_"
            var promptY = y + barH + h * 0.12;
            ctx.fillStyle = "rgba(255,255,255,0.5)";
            ctx.font = "bold " + (h * 0.18) + "px monospace";
            ctx.textAlign = "left";
            ctx.textBaseline = "top";
            ctx.fillText(">_", x + w * 0.12, promptY);

            // Code lines
            var lineY = promptY + h * 0.28;
            var lineH = h * 0.05;
            for (var i = 0; i < 2; i++) {
                var lw = (i === 1) ? w * 0.35 : w * 0.55;
                roundedRect(ctx, x + w * 0.12, lineY + i * h * 0.1, lw, lineH, lineH / 2, "rgba(255,255,255,0.25)", null);
            }
        }

        // --- Desktop file icon (app launcher with rocket) ---
        function drawDesktopIcon(ctx, x, y, w, h, color1, color2) {
            var r = Math.max(1, Math.min(w, h) * 0.08);
            var grad = ctx.createLinearGradient(x, y, x, y + h);
            grad.addColorStop(0, color1);
            grad.addColorStop(1, color2);
            roundedRect(ctx, x, y, w, h, r, grad, "rgba(255,255,255,0.2)", 0.5);

            // Rocket body
            var cx = x + w * 0.5;
            var cy = y + h * 0.45;
            var rw = w * 0.12;
            var rh = h * 0.35;

            ctx.beginPath();
            ctx.moveTo(cx, cy - rh);
            ctx.quadraticCurveTo(cx + rw, cy - rh * 0.5, cx + rw, cy);
            ctx.lineTo(cx + rw, cy + rh * 0.3);
            ctx.lineTo(cx - rw, cy + rh * 0.3);
            ctx.lineTo(cx - rw, cy);
            ctx.quadraticCurveTo(cx - rw, cy - rh * 0.5, cx, cy - rh);
            ctx.closePath();
            ctx.fillStyle = "rgba(255,255,255,0.45)";
            ctx.fill();

            // Window
            ctx.beginPath();
            ctx.arc(cx, cy - rh * 0.2, rw * 0.4, 0, Math.PI * 2);
            ctx.fillStyle = "rgba(0,0,0,0.2)";
            ctx.fill();

            // Fins
            ctx.beginPath();
            ctx.moveTo(cx - rw, cy + rh * 0.1);
            ctx.lineTo(cx - rw * 1.6, cy + rh * 0.45);
            ctx.lineTo(cx - rw, cy + rh * 0.3);
            ctx.closePath();
            ctx.fillStyle = "rgba(255,255,255,0.3)";
            ctx.fill();

            ctx.beginPath();
            ctx.moveTo(cx + rw, cy + rh * 0.1);
            ctx.lineTo(cx + rw * 1.6, cy + rh * 0.45);
            ctx.lineTo(cx + rw, cy + rh * 0.3);
            ctx.closePath();
            ctx.fill();

            // Flame
            ctx.beginPath();
            ctx.moveTo(cx - rw * 0.6, cy + rh * 0.3);
            ctx.lineTo(cx, cy + rh * 0.7);
            ctx.lineTo(cx + rw * 0.6, cy + rh * 0.3);
            ctx.closePath();
            ctx.fillStyle = "rgba(255,200,50,0.5)";
            ctx.fill();
        }

        // --- Grid Folders mode (dynamic count from home dir) ---
        function drawGridFolders(ctx, cw, ch, count, theme) {
            var br = cw * 0.15;
            roundedRect(ctx, 1, 1, cw - 2, ch - 2, br, null, theme.border, 1);

            if (count <= 0) {
                // Draw an empty folder icon
                var fSize = cw * 0.55;
                var fx = (cw - fSize) / 2;
                var fy = (ch - fSize) / 2 - cw * 0.04;
                drawFolderIcon(ctx, fx, fy, fSize, fSize, hexToRgba(theme.colors[0], 0.35), hexToRgba(theme.colors[0], 0.2));
                ctx.fillStyle = hexToRgba(theme.colors[0], 0.5);
                ctx.font = cw * 0.11 + "px sans-serif";
                ctx.textAlign = "center";
                ctx.textBaseline = "middle";
                ctx.fillText(qsTr("Empty"), cw / 2, fy + fSize + cw * 0.1);
                return;
            }

            // Always use 2x2 grid for count >= 2, keeps items square
            var cols = (count === 1) ? 1 : 2;
            var rows = (count === 1) ? 1 : 2;

            var pad = cw * 0.15;
            var gap = cw * 0.06;
            var areaW = cw - pad * 2;
            var areaH = ch - pad * 2;
            var fw = (areaW - gap * (cols - 1)) / cols;
            var fh = (areaH - gap * (rows - 1)) / rows;

            for (var row = 0; row < rows; row++) {
                for (var col = 0; col < cols; col++) {
                    var idx = row * cols + col;
                    if (idx >= count) break;

                    var fx = pad + col * (fw + gap);
                    var fy = pad + row * (fh + gap);
                    var c1 = theme.colors[idx % theme.colors.length];
                    var c2 = darken(c1, 0.65);

                    var entry = Applet.directoryModel ? Applet.directoryModel.get(idx) : null;
                    if (entry && !entry.isDir) {
                        drawFileIcon(ctx, fx, fy, fw, fh, c1, c2);
                    } else {
                        drawFolderIcon(ctx, fx, fy, fw, fh, c1, c2);
                    }
                }
            }
        }

        // --- Count badge (when folders > 4) ---
        function drawCountBadge(ctx, cw, ch, count, theme) {
            var badgeR = cw * 0.14;
            var bx = cw - badgeR - 2;
            var by = badgeR + 2;

            ctx.beginPath();
            ctx.arc(bx, by, badgeR, 0, Math.PI * 2);
            ctx.fillStyle = theme.colors[0];
            ctx.fill();
            ctx.strokeStyle = "rgba(255,255,255,0.5)";
            ctx.lineWidth = 0.8;
            ctx.stroke();

            ctx.fillStyle = "#FFFFFF";
            ctx.font = "bold " + (badgeR * 1.0) + "px sans-serif";
            ctx.textAlign = "center";
            ctx.textBaseline = "middle";
            ctx.fillText(count.toString(), bx, by);
        }

        Connections {
            target: Applet
            function onGridCountChanged() { iconCanvas.requestPaint() }
            function onColorThemeChanged() { iconCanvas.requestPaint() }
            function onFolderCountChanged() { iconCanvas.requestPaint() }
            function onDisplayFolderCountChanged() { iconCanvas.requestPaint() }
        }
    }

    // --- File browser popup ---
    PanelPopup {
        id: filePopup
        popupX: DockPanelPositioner.x
        popupY: DockPanelPositioner.y

        Rectangle {
            id: popupFrame
            width: 480
            height: 400
            color: "transparent"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                // --- Toolbar ---
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Button {
                        flat: true
                        implicitWidth: 32
                        implicitHeight: 28
                        text: "←"
                        font.pixelSize: 16
                        enabled: Applet.canGoBack
                        onClicked: Applet.goBack()
                    }

                    Button {
                        visible: false
                        flat: true
                        implicitWidth: 32
                        implicitHeight: 28
                        text: "→"
                        font.pixelSize: 16
                        enabled: Applet.canGoForward
                        onClicked: Applet.goForward()
                    }

                    Label {
                        visible: false
                        Layout.fillWidth: true
                        text: Applet.directoryPath
                        elide: Text.ElideMiddle
                        font.pixelSize: 12
                        opacity: 0.6
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Button {
                        flat: true
                        visible: false
                        implicitWidth: 32
                        implicitHeight: 28
                        text: "↻"
                        font.pixelSize: 16
                        onClicked: Applet.refreshDirectory()
                    }

                    Button {
                        flat: true
                        visible: false
                        implicitWidth: 32
                        implicitHeight: 28
                        text: Applet.iconViewMode === 0 ? "☰" : "⊞"
                        font.pixelSize: 14
                        onClicked: Applet.iconViewMode = Applet.iconViewMode === 0 ? 1 : 0
                    }
                }

                // --- File grid (large icon mode) ---
                GridView {
                    id: fileGrid
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    visible: Applet.iconViewMode === 0
                    cellWidth: 90
                    cellHeight: 100

                    model: Applet.directoryModel
                    currentIndex: -1

                    onVisibleChanged: {
                        if (visible) forceActiveFocus()
                    }

                    highlight: Rectangle {
                        color: Qt.rgba(0, 0, 0, 0.08)
                        radius: 6
                    }
                    highlightFollowsCurrentItem: true

                    delegate: Item {
                        width: fileGrid.cellWidth
                        height: fileGrid.cellHeight

                        property bool hovered: gridMouseArea.containsMouse

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            radius: 6
                            color: hovered ? Qt.rgba(0, 0, 0, 0.06) : "transparent"
                            visible: hovered
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        Column {
                            anchors.fill: parent
                            anchors.topMargin: 6
                            spacing: 4

                            Image {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 52
                                height: 52
                                source: model.iconUrl
                                sourceSize.width: 52
                                sourceSize.height: 52
                                fillMode: Image.PreserveAspectFit
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: parent.width - 8
                                text: model.fileName
                                font.pixelSize: 11
                                elide: Text.ElideRight
                                horizontalAlignment: Text.AlignHCenter
                                maximumLineCount: 2
                                wrapMode: Text.Wrap
                            }
                        }

                        MouseArea {
                            id: gridMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: fileGrid.currentIndex = index
                            onDoubleClicked: {
                                if (model.isDir) {
                                    Applet.navigateTo(model.filePath)
                                } else {
                                    Applet.openFile(model.filePath)
                                }
                            }
                        }
                    }

                    Keys.onLeftPressed: moveCurrentIndexLeft()
                    Keys.onRightPressed: moveCurrentIndexRight()
                    Keys.onUpPressed: moveCurrentIndexUp()
                    Keys.onDownPressed: moveCurrentIndexDown()
                    Keys.onReturnPressed: {
                        if (currentIndex >= 0) {
                            var entry = Applet.directoryModel.get(currentIndex)
                            if (entry.isDir)
                                Applet.navigateTo(entry.filePath)
                            else
                                Applet.openFile(entry.filePath)
                        }
                    }
                    Keys.onEscapePressed: filePopup.close()

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }

                // --- File list (small icon mode) ---
                ListView {
                    id: fileList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    visible: Applet.iconViewMode === 1

                    model: Applet.directoryModel
                    currentIndex: -1

                    onVisibleChanged: {
                        if (visible) forceActiveFocus()
                    }

                    highlight: Rectangle {
                        color: Qt.rgba(0, 0, 0, 0.08)
                        radius: 4
                    }
                    highlightFollowsCurrentItem: true

                    delegate: Item {
                        width: fileList.width
                        height: 36

                        property bool hovered: listMouseArea.containsMouse

                        Rectangle {
                            anchors.fill: parent
                            anchors.leftMargin: 2
                            anchors.rightMargin: 2
                            radius: 4
                            color: hovered ? Qt.rgba(0, 0, 0, 0.06) : "transparent"
                            visible: hovered
                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 8

                            Image {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 24
                                height: 24
                                source: model.iconUrl
                                sourceSize.width: 24
                                sourceSize.height: 24
                                fillMode: Image.PreserveAspectFit
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.width - 40
                                text: model.fileName
                                font.pixelSize: 12
                                elide: Text.ElideRight
                            }
                        }

                        MouseArea {
                            id: listMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: fileList.currentIndex = index
                            onDoubleClicked: {
                                if (model.isDir) {
                                    Applet.navigateTo(model.filePath)
                                } else {
                                    Applet.openFile(model.filePath)
                                }
                            }
                        }
                    }

                    Keys.onUpPressed: decrementCurrentIndex()
                    Keys.onDownPressed: incrementCurrentIndex()
                    Keys.onReturnPressed: {
                        if (currentIndex >= 0) {
                            var entry = Applet.directoryModel.get(currentIndex)
                            if (entry.isDir)
                                Applet.navigateTo(entry.filePath)
                            else
                                Applet.openFile(entry.filePath)
                        }
                    }
                    Keys.onEscapePressed: filePopup.close()

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }
            }
        }
    }

    // --- Context menu ---
    Loader {
        id: contextMenuLoader
        active: false
        sourceComponent: LP.Menu {
            LP.Menu {
                title: qsTr("Color Theme")
                LP.MenuItem { text: (Applet.colorTheme === 0 ? "✓ " : "    ") + qsTr("Ocean");   onTriggered: Applet.colorTheme = 0 }
                LP.MenuItem { text: (Applet.colorTheme === 1 ? "✓ " : "    ") + qsTr("Sunset");  onTriggered: Applet.colorTheme = 1 }
                LP.MenuItem { text: (Applet.colorTheme === 2 ? "✓ " : "    ") + qsTr("Aurora");  onTriggered: Applet.colorTheme = 2 }
                LP.MenuItem { text: (Applet.colorTheme === 3 ? "✓ " : "    ") + qsTr("Neon");    onTriggered: Applet.colorTheme = 3 }
            }
        }
    }

    // --- Mouse handler ---
    MouseArea {
        id: mouseHandler
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: true

        onClicked: function (mouse) {
            if (mouse.button === Qt.RightButton) {
                toolTip.close()
                contextMenuLoader.active = true
                MenuHelper.openMenu(contextMenuLoader.item)
            } else {
                var point = root.mapToItem(null, root.width / 2, root.height / 2)
                filePopup.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, popupFrame.width, popupFrame.height)
                filePopup.open()
            }
        }

        onHoveredChanged: {
            if (mouseHandler.hovered) {
                var point = root.mapToItem(null, root.width / 2, root.height / 2)
                toolTip.DockPanelPositioner.bounding = Qt.rect(point.x, point.y, toolTip.width, toolTip.height)
                toolTip.open()
            } else {
                toolTip.close()
            }
        }
    }

    // --- Drop area for external folder drag ---
    DropArea {
        id: dropArea
        anchors.fill: parent
        keys: ["text/uri-list"]

        property bool dragHovering: false

        onEntered: function (drag) {
            if (drag.urls.length > 0) {
                drag.accepted = true
                dragHovering = true
            } else {
                drag.accepted = false
            }
        }

        onExited: {
            dragHovering = false
        }

        onDropped: function (drop) {
            dragHovering = false
            if (drop.urls.length === 0)
                return
            var localPath = drop.urls[0].toString().replace(/^file:\/\//, "")
            if (!localPath)
                return
            if (Applet.isDirectory(localPath) || Applet.isFile(localPath)) {
                var dir = Applet.isDirectory(localPath) ? localPath : localPath.substring(0, localPath.lastIndexOf("/"))
                if (dir)
                    Applet.navigateTo(dir)
            }
            drop.accepted = true
        }

        Rectangle {
            anchors.fill: parent
            radius: 8
            color: "transparent"
            border.width: 2
            border.color: Qt.rgba(0, 0.7, 1, 0.6)
            visible: dropArea.dragHovering

            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                radius: 6
                color: Qt.rgba(0, 0.7, 1, 0.15)
            }
        }
    }
}
