/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CANVASVIEWBROKER_H
#define CANVASVIEWBROKER_H

#include "ddplugin_canvas_global.h"

#include <QObject>

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasManager;
class CanvasViewBrokerPrivate;
class CanvasViewBroker : public QObject
{
    Q_OBJECT
    friend class CanvasViewBrokerPrivate;
public:
    explicit CanvasViewBroker(CanvasManager *, QObject *parent = nullptr);
    bool init();
public slots:
    void visualRect(int idx, const QUrl &url, QRect *rect);
    void refresh(int idx);
    void update(int idx);
    void select(const QList<QUrl> &urls);
    void selectedUrls(QList<QUrl> *urls);
private:
    CanvasViewBrokerPrivate *d;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASVIEWBROKER_H
