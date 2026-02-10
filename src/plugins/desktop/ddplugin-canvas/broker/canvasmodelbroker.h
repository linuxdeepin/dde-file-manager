// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void refresh(bool global, int ms, bool updateFile = true);
    bool fetch(const QUrl &url);
    bool take(const QUrl &url);
private:
    CanvasProxyModel *model = nullptr;
};

}

#endif // CANVASMODELBROKER_H
