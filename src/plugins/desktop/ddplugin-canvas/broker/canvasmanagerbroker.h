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
#ifndef CANVASMANAGERBROKER_H
#define CANVASMANAGERBROKER_H

#include "ddplugin_canvas_global.h"

#include <QObject>

class QAbstractItemModel;

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasManager;
class CanvasManagerBrokerPrivate;
class CanvasManagerBroker : public QObject
{
    Q_OBJECT
public:
    explicit CanvasManagerBroker(CanvasManager *canvas, QObject *parent = nullptr);
    bool init();
signals:

public slots:
    void update();
    void edit(const QUrl &url);
    void fileInfoModel(QAbstractItemModel **model);
private:
    CanvasManagerBrokerPrivate *d;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASMANAGERBROKER_H
