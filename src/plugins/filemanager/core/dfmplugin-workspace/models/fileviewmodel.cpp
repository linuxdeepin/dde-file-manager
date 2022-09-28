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

#include "fileviewmodel.h"
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "utils/fileoperatorhelper.h"
#include "utils/filedatahelper.h"
#include "events/workspaceeventsequence.h"
#include "filesortfilterproxymodel.h"

#include "base/application/settings.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/base/application/application.h"

#include <dfm-framework/event/event.h>

#include <QApplication>
#include <QPointer>
#include <QList>
#include <QMimeData>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewModel::FileViewModel(QAbstractItemView *parent)
    : QAbstractItemModel(parent),
      fileDataHelper(new FileDataHelper(this))

{
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::requestFileUpdate, this, &FileViewModel::onFileUpdated);
}

FileViewModel::~FileViewModel()
{
}

QModelIndex FileViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row < 0 || column < 0)
            return QModelIndex();
        auto info = fileDataHelper->findRootInfo(row);
        if (!info || !info->data)
            return QModelIndex();

        return createIndex(row, column, info->data);
    }

    auto data = fileDataHelper->findFileItemData(parent.row(), row);
    if (data)
        return createIndex(row, column, data);

    return QModelIndex();
}

QUrl FileViewModel::rootUrl(const QModelIndex &rootIndex) const
{
    auto rootInfo = fileDataHelper->findRootInfo(rootIndex.row());
    if (rootInfo)
        return rootInfo->url;

    return QUrl();
}

QModelIndex FileViewModel::rootIndex(const QUrl &rootUrl) const
{
    const RootInfo *info = fileDataHelper->findRootInfo(rootUrl);
    return createIndex(info->rowIndex, 0, info->data);
}

QModelIndex FileViewModel::setRootUrl(const QUrl &url)
{
    if (!url.isValid())
        return QModelIndex();

    int rootRow = fileDataHelper->preSetRoot(url);
    if (rowCount() <= rootRow) {
        // insert root index
        beginInsertRows(QModelIndex(), rootRow, rootRow);
        RootInfo *root = fileDataHelper->setRoot(url);
        endInsertRows();

        connect(root, &RootInfo::insert, this, &FileViewModel::onInsert, Qt::QueuedConnection);
        connect(root, &RootInfo::insertFinish, this, &FileViewModel::onInsertFinish, Qt::QueuedConnection);
        connect(root, &RootInfo::remove, this, &FileViewModel::onRemove, Qt::QueuedConnection);
        connect(root, &RootInfo::removeFinish, this, &FileViewModel::onRemoveFinish, Qt::QueuedConnection);
        connect(root, &RootInfo::childrenUpdate, this, &FileViewModel::childrenUpdated, Qt::QueuedConnection);
    } else {
        fileDataHelper->setRoot(url);
    }

    const QModelIndex &index = rootIndex(url);
    fetchMore(index);

    return index;
}

QModelIndex FileViewModel::findIndex(const QUrl &url) const
{
    auto indexPair = fileDataHelper->getIndexByUrl(url);

    if (indexPair.first < 0)
        return QModelIndex();

    const QModelIndex &parentIndex = index(indexPair.first, 0, QModelIndex());

    if (indexPair.second < 0)
        return parentIndex;

    return index(indexPair.second, 0, parentIndex);
}

AbstractFileInfoPointer FileViewModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() < 0)
        return nullptr;

    if (!index.parent().isValid()) {
        RootInfo *info = fileDataHelper->findRootInfo(index.row());

        if (info && info->data)
            return info->data->fileInfo();

        return nullptr;
    }

    const QModelIndex &parentIndex = index.parent();
    const FileItemData *data = fileDataHelper->findFileItemData(parentIndex.row(), index.row());
    if (data)
        return data->fileInfo();

    return nullptr;
}

QList<QUrl> FileViewModel::getChildrenUrls(const QUrl &rootUrl) const
{
    const RootInfo *info = fileDataHelper->findRootInfo(rootUrl);
    if (info)
        return info->getChildrenUrls();

    return {};
}

QModelIndex FileViewModel::parent(const QModelIndex &child) const
{
    const FileItemData *childData = static_cast<FileItemData *>(child.internalPointer());

    if (childData && childData->parentData()
        && childData->parentData()->fileInfo()) {
        return findIndex(childData->parentData()->fileInfo()->url());
    }

    return QModelIndex();
}

int FileViewModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return fileDataHelper->rootsCount();

    return fileDataHelper->filesCount(parent.row());
}

int FileViewModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 1;

    const RootInfo *info = fileDataHelper->findRootInfo(parent.row());
    if (info)
        return getColumnRoles(info->url).count();

    return 1;
}

QVariant FileViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.parent().isValid()) {
        RootInfo *info = fileDataHelper->findRootInfo(index.row());
        if (info) {
            QVariant data;
            if (WorkspaceEventSequence::instance()->doFetchCustomRoleData(rootUrl(index), info->url, static_cast<ItemRoles>(role), &data))
                return data;
            return info->data->data(role);
        }

        return QVariant();
    }

    FileItemData *itemData = fileDataHelper->findFileItemData(index.parent().row(), index.row());

    if (itemData && itemData->fileInfo()) {
        QVariant data;
        if (WorkspaceEventSequence::instance()->doFetchCustomRoleData(rootUrl(index), itemData->fileInfo()->url(), static_cast<ItemRoles>(role), &data))
            return data;
        return itemData->data(role);
    }

    return QVariant();
}

void FileViewModel::clear(const QUrl &rootUrl)
{
    fileDataHelper->clear(rootUrl);
}

void FileViewModel::update(const QUrl &rootUrl)
{
    fileDataHelper->update(rootUrl);

    RootInfo *info = fileDataHelper->findRootInfo(rootUrl);
    if (info) {
        const QModelIndex &parentIndex = findIndex(rootUrl);
        emit dataChanged(index(0, 0, parentIndex), index(info->childrenCount(), 0, parentIndex));
    }
}

void FileViewModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent)) {
        QApplication::restoreOverrideCursor();
        return;
    }

    const QUrl &url = rootUrl(parent);
    if (WorkspaceHelper::instance()->haveViewRoutePrehandler(url.scheme())) {
        Q_EMIT traverPrehandle(url, parent);
    } else {
        traversRootDir(parent);
    }
}

bool FileViewModel::canFetchMore(const QModelIndex &parent) const
{
    auto rootInfo = fileDataHelper->findRootInfo(parent.row());
    if (rootInfo)
        return rootInfo->canFetchMore;

    return false;
}

Qt::ItemFlags FileViewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    const AbstractFileInfoPointer &info = fileInfo(index);
    if (!info)
        return flags;

    if (info->canRename())
        flags |= Qt::ItemIsEditable;

    if (info->isWritable()) {
        if (info->canDrop())
            flags |= Qt::ItemIsDropEnabled;
        else
            flags |= Qt::ItemNeverHasChildren;
    }

    if (info->canDrag())
        flags |= Qt::ItemIsDragEnabled;

    return flags;
}

QStringList FileViewModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

QMimeData *FileViewModel::mimeData(const QModelIndexList &indexes) const
{
    QList<QUrl> urls;
    QSet<QUrl> urlsSet;
    QList<QModelIndex>::const_iterator it = indexes.begin();

    for (; it != indexes.end(); ++it) {
        if ((*it).column() == 0) {
            const AbstractFileInfoPointer &fileInfo = this->fileInfo(*it);
            const QUrl &url = fileInfo->url();

            if (urlsSet.contains(url))
                continue;

            urls << url;
            urlsSet << url;
        }
    }

    QMimeData *data = new QMimeData();
    data->setUrls(urls);

    QByteArray userID;
    userID.append(QString::number(SysInfoUtils::getUserId()));
    data->setData(DFMGLOBAL_NAMESPACE::Mime::kMimeDataUserIDKey, userID);

    return data;
}

bool FileViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    const QModelIndex &dropIndex = index(row, column, parent);

    if (!dropIndex.isValid())
        return false;

    const AbstractFileInfoPointer &targetFileInfo = fileInfo(dropIndex);
    QUrl targetUrl = targetFileInfo->url();
    QList<QUrl> dropUrls = data->urls();
    QList<QUrl> urls {};
    bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", dropUrls, &urls);
    if (ok && !urls.isEmpty())
        dropUrls = urls;

    if (targetFileInfo->isSymLink())
        targetUrl = QUrl::fromLocalFile(targetFileInfo->symLinkTarget());

    FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());

    if (FileUtils::isTrashDesktopFile(targetUrl)) {
        FileOperatorHelperIns->moveToTrash(view, dropUrls);
        return true;
    } else if (FileUtils::isDesktopFile(targetUrl)) {
        FileOperatorHelperIns->openFilesByApp(view, dropUrls, QStringList { targetUrl.toLocalFile() });
        return true;
    }

    bool ret { true };

    switch (action) {
    case Qt::CopyAction:
        if (dropUrls.count() > 0) {
            // call copy
            FileOperatorHelperIns->dropFiles(view, Qt::CopyAction, targetUrl, dropUrls);
        }
        break;
    case Qt::MoveAction:
        if (dropUrls.count() > 0) {
            // call move
            FileOperatorHelperIns->dropFiles(view, Qt::MoveAction, targetUrl, dropUrls);
        }
        break;
    default:
        break;
    }

    return ret;
}

void FileViewModel::traversRootDir(const QModelIndex &rootIndex)
{
    if (rootIndex.isValid()) {
        Q_EMIT stateChanged(rootUrl(rootIndex), ModelState::kBusy);
        fileDataHelper->doTravers(rootIndex.row());
    }
}

void FileViewModel::stopTraversWork(const QUrl &rootUrl)
{
    fileDataHelper->doStopWork(rootUrl);
    Q_EMIT stateChanged(rootUrl, ModelState::kIdle);
}

QList<ItemRoles> FileViewModel::getColumnRoles(const QUrl &rootUrl) const
{
    QList<ItemRoles> roles;
    bool customOnly = WorkspaceEventSequence::instance()->doFetchCustomColumnRoles(rootUrl, &roles);

    const QVariantMap &map = DFMBASE_NAMESPACE::Application::appObtuselySetting()->value("FileViewState", rootUrl).toMap();
    if (map.contains("headerList")) {
        QVariantList headerList = map.value("headerList").toList();

        for (ItemRoles role : roles) {
            if (!headerList.contains(role))
                headerList.append(role);
        }

        roles.clear();
        for (auto var : headerList) {
            roles.append(static_cast<ItemRoles>(var.toInt()));
        }
    } else if (!customOnly) {
        static QList<ItemRoles> defualtColumnRoleList = QList<ItemRoles>() << kItemFileDisplayNameRole
                                                                           << kItemFileLastModifiedRole
                                                                           << kItemFileSizeRole
                                                                           << kItemFileMimeTypeRole;

        int customCount = roles.count();
        for (auto role : defualtColumnRoleList) {
            if (!roles.contains(role))
                roles.insert(roles.length() - customCount, role);
        }
    }

    return roles;
}

void FileViewModel::setIndexActive(const QModelIndex &index, bool enable)
{
    fileDataHelper->setFileActive(index.parent().row(), index.row(), enable);
}

void FileViewModel::onFilesUpdated()
{
    FileView *view = qobject_cast<FileView *>(qobject_cast<QObject *>(this)->parent());
    if (view) {
        QDir::Filters filter = view->model()->getFilters();
        view->model()->setFilters(filter);
    }
    emit updateFiles();
}

void FileViewModel::onFileUpdated(const QUrl &url)
{
    const QModelIndex &index = findIndex(url);
    if (index.isValid()) {
        auto info = InfoFactory::create<AbstractFileInfo>(url);
        if (info)
            info->refresh();

        emit dataChanged(index, index);
    }
}

void FileViewModel::onInsert(int rootIndex, int firstIndex, int count)
{
    const QModelIndex &parentIndex = index(rootIndex, 0, QModelIndex());

    beginInsertRows(parentIndex, firstIndex, firstIndex + count - 1);
}

void FileViewModel::onInsertFinish()
{
    endInsertRows();
}

void FileViewModel::onRemove(int rootIndex, int firstIndex, int count)
{
    const QModelIndex &parentIndex = index(rootIndex, 0, QModelIndex());

    beginRemoveRows(parentIndex, firstIndex, firstIndex + count - 1);
}

void FileViewModel::onRemoveFinish()
{
    endRemoveRows();
}
