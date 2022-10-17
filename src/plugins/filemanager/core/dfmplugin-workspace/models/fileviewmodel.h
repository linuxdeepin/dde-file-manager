/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

    QModelIndex setRootUrl(const QUrl &url);

    void clear(const QUrl &rootUrl);
    void update(const QUrl &rootUrl);

    AbstractFileInfoPointer fileInfo(const QModelIndex &index) const;
    AbstractFileInfoPointer fileInfo(const QModelIndex &parent, const QModelIndex &index) const;
    QList<QUrl> getChildrenUrls(const QUrl &rootUrl) const;

    QModelIndex findIndex(const QUrl &url) const;

    void traversRootDir(const QModelIndex &rootIndex);
    void stopTraversWork(const QUrl &rootUrl);

    QList<DFMGLOBAL_NAMESPACE::ItemRoles> getColumnRoles(const QUrl &rootUrl) const;

    void setIndexActive(const QModelIndex &index, bool enable);

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
    void updateFiles();
    void selectAndEditFile(const QUrl &url);
    void traverPrehandle(const QUrl &url, const QModelIndex &index);

private:
    FileDataHelper *fileDataHelper;
};

}

#endif   // FILEVIEWMODEL_H
