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
#ifndef CANVASMODELBROKER_H
#define CANVASMODELBROKER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QUrl>
#include <QModelIndex>
#include <QVariant>

namespace ddplugin_canvas {
class CanvasProxyModel;
class CanvasModelBroker : public QObject
{
    Q_OBJECT
public:
    explicit CanvasModelBroker(CanvasProxyModel *model, QObject *parent = nullptr);
    ~CanvasModelBroker();
    bool init();
public slots:
    // model interfaces
    QUrl rootUrl();
    QModelIndex urlIndex(const QUrl &url);
    QModelIndex index(int row);
    QUrl fileUrl(const QModelIndex &index);
    QList<QUrl> files();
    bool showHiddenFiles();
    void setShowHiddenFiles(bool show);
    int sortOrder();
    void setSortOrder(int order);
    int sortRole();
    void setSortRole(int role, int order);
    int rowCount();
    QVariant data(const QUrl &url, int itemRole);
    void sort();
    void refresh(bool global, int ms);
    bool fetch(const QUrl &url);
    bool take(const QUrl &url);
private:
    CanvasProxyModel *model = nullptr;
};

}

#endif // CANVASMODELBROKER_H
