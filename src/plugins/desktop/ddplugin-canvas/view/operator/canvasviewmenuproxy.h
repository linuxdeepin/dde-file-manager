// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASVIEWMENUPROXY_H
#define CANVASVIEWMENUPROXY_H

#include "ddplugin_canvas_global.h"

#include <DMenu>

#include <QObject>

namespace ddplugin_canvas {
class CanvasView;
class CanvasViewMenuProxy : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewMenuProxy(CanvasView *parent = nullptr);
    ~CanvasViewMenuProxy();
    static bool disableMenu();
    void showEmptyAreaMenu(const Qt::ItemFlags &indexFlags, const QPoint gridPos);
    void showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos);
public slots:
    void changeIconLevel(bool increase);

private:
    CanvasView *view { nullptr };
    DTK_WIDGET_NAMESPACE::DMenu *menuPtr { nullptr };
};

}

#endif   // CANVASVIEWMENUPROXY_H
