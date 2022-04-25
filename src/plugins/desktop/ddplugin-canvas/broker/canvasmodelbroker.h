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

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasProxyModel;
class CanvasModelBrokerPrivate;
class CanvasModelBroker : public QObject
{
    Q_OBJECT
    friend class CanvasModelBrokerPrivate;
public:
    explicit CanvasModelBroker(CanvasProxyModel *model, QObject *parent = nullptr);
    bool init();
public slots:
    // model interfaces
    void rootUrl(QUrl *url);
    void urlIndex(const QUrl &url, QModelIndex *idx);
    void index(int row, QModelIndex *idx);
    void fileUrl(const QModelIndex &index, QUrl *url);
    void files(QList<QUrl> *urls);
    void showHiddenFiles(bool *show);
    void setShowHiddenFiles(bool show);
    void sortOrder(int *order);
    void setSortOrder(int order);
    void sortRole(int *role);
    void setSortRole(int role, int order);
    void rowCount(int *count);
    void data(const QUrl &url, int itemRole, QVariant *out);
    void sort();
    void refresh(bool global, int ms);
    void fetch(const QUrl &url, bool *ret);
    void take(const QUrl &url, bool *ret);
private:
    CanvasModelBrokerPrivate *d;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASMODELBROKER_H
