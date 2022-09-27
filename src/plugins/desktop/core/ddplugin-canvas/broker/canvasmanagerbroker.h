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
class QAbstractItemView;

namespace ddplugin_canvas {
class CanvasManager;
class CanvasManagerBroker : public QObject
{
    Q_OBJECT
public:
    explicit CanvasManagerBroker(CanvasManager *canvas, QObject *parent = nullptr);
    ~CanvasManagerBroker();
    bool init();
public slots:
    void update();
    void edit(const QUrl &url);
    int iconLevel();
    void setIconLevel(int lv);
    bool autoArrange();
    void setAutoArrange(bool on);
    QAbstractItemModel *fileInfoModel();
    QAbstractItemView *view(int viewIdx);
private:
    CanvasManager *canvas = nullptr;
};

}

#endif // CANVASMANAGERBROKER_H
