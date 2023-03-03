// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWMODEL_H
#define FILEVIEWMODEL_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QUrl>

#include <iostream>
#include <memory>

#include <unistd.h>

class QAbstractItemView;

namespace dfmplugin_workspace {

class FileItemData;
class FileDataHelper;
class FileView;
class FileViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FileViewModel(QAbstractItemView *parent = nullptr);
    virtual ~FileViewModel() override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    QUrl rootUrl(const QModelIndex &rootIndex) const;
    QModelIndex rootIndex(const QUrl &rootUrl) const;

    QModelIndex setRootUrl(const QUrl &url, FileView *view);

    void clear(const QUrl &rootUrl);
    void update(const QUrl &rootUrl);
    void refresh(const QUrl &rootUrl);

    AbstractFileInfoPointer fileInfo(const QModelIndex &index) const;
    AbstractFileInfoPointer fileInfo(const QModelIndex &parent, const QModelIndex &index) const;
    QList<QUrl> getChildrenUrls(const QUrl &rootUrl) const;

    QModelIndex findRootIndex(const QUrl &url) const;
    QModelIndex findChildIndex(const QUrl &rootUrl, const QUrl &url) const;

    void doFetchMore(const QModelIndex &rootIndex);
    void traversRootDir(const QModelIndex &rootIndex);
    void stopTraversWork(const QUrl &rootUrl);

    QList<DFMGLOBAL_NAMESPACE::ItemRoles> getColumnRoles(const QUrl &rootUrl) const;

    void setIndexActive(const QModelIndex &index, bool enable);

    void cleanDataCacheByUrl(const QUrl &url);
    void updateRoot(const QList<QUrl> urls);

    void updateFile(const QUrl &root, const QUrl &url);

public Q_SLOTS:
    void onFilesUpdated();
    void onFileUpdated(const QUrl &url);
    void onInsert(int rootIndex, int firstIndex, int count);
    void onInsertFinish();
    void onRemove(int rootIndex, int firstIndex, int count);
    void onRemoveFinish();

Q_SIGNALS:
    void stateChanged(const QUrl &url, ModelState state);
    void childrenUpdated(const QUrl &url);
    void selectAndEditFile(const QUrl &rootUrl, const QUrl &url);
    void updateFiles();
    void traverPrehandle(const QUrl &url, const QModelIndex &index, FileView *view);
    void reloadView();

private:
    FileDataHelper *fileDataHelper;
};

}

#endif   // FILEVIEWMODEL_H
