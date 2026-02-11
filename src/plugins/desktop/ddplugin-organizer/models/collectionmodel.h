// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONMODEL_H
#define COLLECTIONMODEL_H

#include "ddplugin_organizer_global.h"

#include <dfm-base/file/local/syncfileinfo.h>

#include <QAbstractProxyModel>

namespace ddplugin_organizer {
class ModelDataHandler;
class CanvasViewShell;
class FileInfoModelShell;
class CollectionModelPrivate;
class CollectionModel : public QAbstractProxyModel
{
    Q_OBJECT
    friend class CollectionModelPrivate;

public:
    explicit CollectionModel(QObject *parent = nullptr);
    ~CollectionModel() override;
    void setModelShell(FileInfoModelShell *shell);
    FileInfoModelShell *modelShell() const;
    void setHandler(ModelDataHandler *);
    ModelDataHandler *handler() const;
    QUrl rootUrl() const;
    QModelIndex rootIndex() const;
    QModelIndex index(const QUrl &url, int column = 0) const;
    FileInfoPointer fileInfo(const QModelIndex &index) const;
    QList<QUrl> files() const;
    QUrl fileUrl(const QModelIndex &index) const;
    void refresh(const QModelIndex &parent, bool global = false, int ms = 50, bool file = true);
    void update();
    bool fetch(const QList<QUrl> &urls);
    bool take(const QList<QUrl> &urls);

public:
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
signals:
    void dataReplaced(const QUrl &oldUrl, const QUrl &newUrl);

private:
    void setSourceModel(QAbstractItemModel *sourceModel) override;

private:
    CollectionModelPrivate *d = nullptr;
};

}

#endif   // COLLECTIONMODEL_H
