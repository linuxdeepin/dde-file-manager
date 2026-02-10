// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASPROXYMODEL_H
#define CANVASPROXYMODEL_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/file/local/syncfileinfo.h>

#include <QAbstractProxyModel>
#include <QDir>

namespace ddplugin_canvas {

class ModelHookInterface;
class CanvasProxyModelPrivate;
class CanvasProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    friend class CanvasProxyModelPrivate;
    friend class CanvasModelBroker;

public:
    explicit CanvasProxyModel(QObject *parent = nullptr);
    QModelIndex rootIndex() const;
    inline QUrl rootUrl() const { return fileUrl(rootIndex()); }
    QModelIndex index(const QUrl &url, int column = 0) const;
    FileInfoPointer fileInfo(const QModelIndex &index) const;
    QUrl fileUrl(const QModelIndex &index) const;
    QList<QUrl> files() const;
    bool showHiddenFiles() const;
    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder &order);

    int sortRole() const;
    void setSortRole(int role, Qt::SortOrder order = Qt::AscendingOrder);

    void setModelHook(ModelHookInterface *);
    ModelHookInterface *modelHook() const;

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
    void refresh(const QModelIndex &parent, bool global = false, int ms = 50, bool updateFile = true);
    void setShowHiddenFiles(bool show);
    bool fetch(const QUrl &url);   //show \a url if exsited
    bool take(const QUrl &url);   // hide \a url
private:
    CanvasProxyModelPrivate *d = nullptr;
};

}

#endif   // CANVASPROXYMODEL_H
