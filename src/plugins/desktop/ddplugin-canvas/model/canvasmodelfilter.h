// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMODELFILTER_H
#define CANVASMODELFILTER_H

#include "ddplugin_canvas_global.h"
#include "canvasproxymodel.h"

namespace ddplugin_canvas {

class CanvasModelFilter
{
    Q_DISABLE_COPY(CanvasModelFilter)
public:
    explicit CanvasModelFilter(CanvasProxyModel *m);
    virtual bool insertFilter(const QUrl &url);
    virtual bool resetFilter(QList<QUrl> &urls);
    virtual bool updateFilter(const QUrl &url, const QVector<int> &roles = {});
    virtual bool removeFilter(const QUrl &url);
    virtual bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl);

protected:
    CanvasProxyModel *model = nullptr;
};

class HiddenFileFilter : public CanvasModelFilter
{
public:
    using CanvasModelFilter::CanvasModelFilter;
    bool insertFilter(const QUrl &url) override;
    bool resetFilter(QList<QUrl> &urls) override;
    bool updateFilter(const QUrl &url, const QVector<int> &roles = {}) override;
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl) override;
};

class InnerDesktopAppFilter : public QObject, public CanvasModelFilter
{
    Q_OBJECT
public:
    explicit InnerDesktopAppFilter(CanvasProxyModel *model, QObject *parent = nullptr);
    void refreshModel();
    bool resetFilter(QList<QUrl> &urls) override;
    bool insertFilter(const QUrl &url) override;
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl) override;
public slots:
    void changed(const QString &key);

protected:
    QMap<QString, QUrl> keys;
    QMap<QString, bool> hidden;
};

class ModelHookInterface;
class HookFilter : public CanvasModelFilter
{
public:
    using CanvasModelFilter::CanvasModelFilter;
    bool insertFilter(const QUrl &url) override;
    bool resetFilter(QList<QUrl> &urls) override;
    bool updateFilter(const QUrl &url, const QVector<int> &roles = {}) override;
    bool removeFilter(const QUrl &url) override;
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl) override;
};
}

#endif   // CANVASMODELFILTER_H
