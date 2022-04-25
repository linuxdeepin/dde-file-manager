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
#ifndef CANVASPROXYMODEL_H
#define CANVASPROXYMODEL_H

#include "ddplugin_canvas_global.h"

#include <file/local/localfileinfo.h>

#include <QAbstractProxyModel>
#include <QDir>

DDP_CANVAS_BEGIN_NAMESPACE

class ModelExtendInterface;
class CanvasProxyModelPrivate;
class CanvasProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    friend class CanvasProxyModelPrivate;
    friend class CanvasModelBroker;
public:
    explicit CanvasProxyModel(QObject *parent = nullptr);
    QModelIndex rootIndex() const;
    inline QUrl rootUrl() const {return fileUrl(rootIndex());}
    QModelIndex index(const QUrl &url, int column = 0) const;
    DFMLocalFileInfoPointer fileInfo(const QModelIndex &index) const;
    QUrl fileUrl(const QModelIndex &index) const;
    QList<QUrl> files() const;
    bool showHiddenFiles() const;
    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder &order);

    int sortRole() const;
    void setSortRole(int role, Qt::SortOrder order = Qt::AscendingOrder);

    void setModelExtend(ModelExtendInterface *);
    ModelExtendInterface *modelExtend() const;
public:
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    Q_INVOKABLE virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    Q_INVOKABLE virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex index(int row, int column = 0,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int itemRole = Qt::DisplayRole) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
signals:
    void dataReplaced(const QUrl &oldUrl, const QUrl &newUrl);
public slots:
    bool sort();
    void update();
    void refresh(const QModelIndex &parent, bool global = false, int ms = 50);
    void setShowHiddenFiles(bool show);
    bool fetch(const QUrl &url); //show \a url if exsited
    bool take(const QUrl &url);  // hide \a url
private:
    CanvasProxyModelPrivate *d;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASPROXYMODEL_H
