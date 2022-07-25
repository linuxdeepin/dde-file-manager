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
#ifndef CANVASMODELFILTER_H
#define CANVASMODELFILTER_H

#include "ddplugin_canvas_global.h"
#include "canvasproxymodel.h"

class QGSettings;

namespace ddplugin_canvas {

class CanvasModelFilter
{
    Q_DISABLE_COPY(CanvasModelFilter)
public:
    explicit CanvasModelFilter(CanvasProxyModel *m);
    virtual bool insertFilter(const QUrl &url);
    virtual bool resetFilter(QList<QUrl> &urls);
    virtual bool updateFilter(const QUrl &url);
    virtual bool removeFilter(const QUrl &url);
    virtual bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl);
protected:
    CanvasProxyModel *model;
};

class HiddenFileFilter : public CanvasModelFilter
{
public:
    using CanvasModelFilter::CanvasModelFilter;
    bool insertFilter(const QUrl &url) override;
    bool resetFilter(QList<QUrl> &urls) override;
    bool updateFilter(const QUrl &url) override;
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl) override;
};

class InnerDesktopAppFilter : public QObject, public CanvasModelFilter
{
    Q_OBJECT
public:
    explicit InnerDesktopAppFilter(CanvasProxyModel *model, QObject *parent = nullptr);
    void update();
    void refreshModel();
    bool resetFilter(QList<QUrl> &urls) override;
    bool insertFilter(const QUrl &url) override;
    bool renameFilter(const QUrl &oldUrl, const QUrl &newUrl) override;
public slots:
    void changed(const QString &key);
protected:
    QGSettings *gsettings = nullptr;
    QMap<QString, QUrl> keys;
    QMap<QString, bool> hidden;
};
}

#endif // CANVASMODELFILTER_H
