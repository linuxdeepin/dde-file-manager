// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTOPER_H
#define SHORTCUTOPER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QKeyEvent>

namespace ddplugin_canvas {
class CanvasView;
class ShortcutOper : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutOper(CanvasView *parent);
    bool keyPressed(QKeyEvent *event);
    bool disableShortcut() const;
protected slots:
    void helpAction();
    void tabToFirst();
    void showMenu();
    void clearClipBoard();
    void switchHidden();
    void previewFiles();

protected:
    CanvasView *view = nullptr;
};

}

#endif   // SHORTCUTOPER_H
