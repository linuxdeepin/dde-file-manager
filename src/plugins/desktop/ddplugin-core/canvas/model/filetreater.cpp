/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "filetreater.h"
#include "utils/fileutil.h"

#include "filefilter.h"

#include <QApplication>
#include <QDebug>

DSB_D_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

FileTreater::FileTreater(CanvasModel *parent)
    : QObject (parent)
{
    qRegisterMetaType<QList<QUrl>>();

    fileFilters << QSharedPointer<FileFilter>(new CustomHiddenFilter(parent));
    fileFilters << QSharedPointer<FileFilter>(new InnerDesktopAppController(parent));
}

FileTreater::~FileTreater()
{

}

bool FileTreater::doSort(QList<DFMLocalFileInfoPointer> &files) const
{
    if (files.isEmpty())
        return true;

    auto firstInfo = files.first();
    AbstractFileInfo::CompareFunction sortFunc = firstInfo->compareFunByKey(fileSortRole);
    if (sortFunc) {
        // standard sort function
        std::sort(files.begin(), files.end(), [sortFunc, this](const DFMLocalFileInfoPointer info1, const DFMLocalFileInfoPointer info2) {
            return sortFunc(info1, info2, fileSortOrder);
        });

        // advanced sort for special case.
        specialSort(files);
        return true;
    } else {
        return false;
    }
}

dfm_service_desktop::CanvasModel *FileTreater::model() const
{
    return static_cast<CanvasModel*>(parent());
}

void FileTreater::insertChild(const QUrl &url)
{
    if (insertChildFilter(url))
        return;

    int row = -1;
    {
        QMutexLocker lk(&childrenMutex);
        if (Q_UNLIKELY(fileList.contains(url))) {
            qInfo() << "file exists:" << url;
            return;
        }
        row = fileList.count();
    }

    auto itemInfo = FileCreator->createFileInfo(url);
    if (Q_UNLIKELY(!itemInfo)) {
        return;
    }

    model()->beginInsertRows(model()->rootIndex(), row, row);
    fileList.append(url);
    fileMap.insert(url, itemInfo);
    model()->endInsertRows();
}

void FileTreater::removeChild(const QUrl &url)
{
    if (removeChildFilter(url))
        return;

    if (Q_UNLIKELY(!fileList.contains(url))) {
        qInfo() << "file dose not exists:" << url;
        return;
    }

    int position = fileList.indexOf(url);
    model()->beginRemoveRows(model()->rootIndex(), position, position);
    {
        QMutexLocker lk(&childrenMutex);
        fileList.removeAt(position);
        fileMap.remove(url);
    }
    model()->endRemoveRows();
}

void FileTreater::renameChild(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (renameChildFilter(oldUrl, newUrl))
        return;

    if (Q_UNLIKELY(!fileList.contains(oldUrl)) || Q_UNLIKELY(fileList.contains(newUrl))) {
        qWarning() << "unknow error in rename file:" << fileList.contains(oldUrl) << oldUrl << fileList.contains(newUrl) << newUrl;
        return;
    }

    if (auto info = FileCreator->createFileInfo(newUrl)) {
        QMutexLocker lk(&childrenMutex);
        int position = fileList.indexOf(oldUrl);
        if (Q_LIKELY(-1 != position))
            fileList.replace(position, newUrl);
        else
            fileList.append(newUrl);

        fileMap.remove(oldUrl);
        fileMap.insert(newUrl, info);

        const QModelIndex &index = model()->index(newUrl.toString());
        model()->dataChanged(index, index);

        model()->fileRenamed(oldUrl, newUrl);
    }
}

void FileTreater::updateChild(const QUrl &url)
{
    if (updateChildFilter(url))
        return;

    if (Q_UNLIKELY(!fileMap.contains(url)))
        return;

    // Although the files cached in InfoCache will be refreshed automatically,
    // a redundant refresh is still required here, because the current variant of LocalFileInfo
    // (like DesktopFileInfo created from DesktopFileCreator) is not in InfoCache and will not be refreshed automatically.
    if (auto info = fileMap.value(url))
        info->refresh();

    const QModelIndex &index = model()->index(url.toString());
    if (Q_UNLIKELY(!index.isValid()))
        return;

    model()->dataChanged(index, index);
}

bool FileTreater::removeChildFilter(const QUrl &url)
{
    for (const auto &filter : fileFilters)
        if (filter->fileDeletedFilter(url))
            return true;

    return false;
}

bool FileTreater::insertChildFilter(const QUrl &url)
{
    for (const auto &filter : fileFilters)
        if (filter->fileCreatedFilter(url))
            return true;

    return false;
}

bool FileTreater::renameChildFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    for (const auto &filter : fileFilters)
        if (filter->fileRenameFilter(oldUrl, newUrl))
            return true;

    return false;
}

bool FileTreater::updateChildFilter(const QUrl &url)
{
    for (const auto &filter : fileFilters)
        if (filter->fileUpdatedFilter(url))
            return true;

    return false;
}

bool FileTreater::traversalFilter(const QUrl &url)
{
    for (const auto &filter : fileFilters)
        if (filter->fileTraversalFilter(url))
            return true;

    return false;
}

DFMLocalFileInfoPointer FileTreater::fileInfo(const QUrl &url)
{
    return fileMap.value(url);
}

DFMLocalFileInfoPointer FileTreater::fileInfo(int index)
{
    if (index >= 0 && index < childrenCount()) {
        QUrl url(fileList.at(index));
        return fileMap.value(url);
    }

    return nullptr;
}

int FileTreater::childrenCount() const
{
    return fileList.count();
}

QList<QUrl> FileTreater::files() const
{
    return fileList;
}

int FileTreater::indexOfChild(AbstractFileInfoPointer info)
{
    return fileList.indexOf(info->url());
}

bool FileTreater::sort()
{
    if (fileList.isEmpty())
        return true;

    auto files = fileMap.values();
    if (!doSort(files)) {
        return false;
    }

    QList<QUrl> tempFileList;
    QMap<QUrl, DFMLocalFileInfoPointer> tempFileMap;
    for (auto itemInfo : files) {
        if (Q_UNLIKELY(!itemInfo))
            continue;

        tempFileList.append(itemInfo->url());
        tempFileMap.insert(itemInfo->url(), itemInfo);
    }

    model()->layoutAboutToBeChanged();
    {
        QModelIndexList from = model()->indexs();
        QMutexLocker lk(&childrenMutex);
        fileList = tempFileList;
        fileMap = tempFileMap;
        QModelIndexList to = model()->indexs();
        model()->changePersistentIndexList(from, to);
    }
    model()->layoutChanged();

    return true;
}

Qt::SortOrder FileTreater::sortOrder() const
{
    return fileSortOrder;
}

void FileTreater::setSortOrder(const Qt::SortOrder order)
{
    if (order == fileSortOrder)
        return;

    fileSortOrder = order;
}

int FileTreater::sortRole() const
{
    return fileSortRole;
}

void FileTreater::setSortRole(const AbstractFileInfo::SortKey role, const Qt::SortOrder order)
{
    if (role != fileSortRole)
        fileSortRole = role;
    if (order != fileSortOrder)
        fileSortOrder = order;
}

void FileTreater::onTraversalFinished()
{
    QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void FileTreater::specialSort(QList<DFMLocalFileInfoPointer> &files) const
{
    if (fileSortRole == AbstractFileInfo::kSortByFileMimeType)
        sortMainDesktopFile(files, fileSortOrder);
}

void FileTreater::sortMainDesktopFile(QList<DFMLocalFileInfoPointer> &files, Qt::SortOrder order) const
{
    // let the main desktop files always on front or back.

    //! warrning: the root url and LocalFileInfo::url must be like file://
    QDir dir(model()->rootUrl().toString());
    QList<QPair<QString, DFMLocalFileInfoPointer>> mainDesktop = {{dir.filePath("dde-home.desktop"), DFMLocalFileInfoPointer()},
        {dir.filePath("dde-trash.desktop"), DFMLocalFileInfoPointer()},
        {dir.filePath("dde-computer.desktop"), DFMLocalFileInfoPointer()}
    };
    auto list = files;
    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        for (const DFMLocalFileInfoPointer &info : list)
        if (info->url().toString() == it->first) {
            it->second = info;
            files.removeOne(info);
        }
    }

    for (auto it = mainDesktop.begin(); it != mainDesktop.end(); ++it) {
        if (it->second) {
            if (order == Qt::AscendingOrder)
                files.push_front(it->second);
            else
                files.push_back(it->second);
        }
    }
}

void FileTreater::onUpdateChildren(const QList<QUrl> &children)
{
    QList<DFMLocalFileInfoPointer> files;
    for (const QUrl &child : children) {
        if (traversalFilter(child))
            continue;

        if (auto itemInfo = FileCreator->createFileInfo(child))
            files.append(itemInfo);
    }

    // defalut sort
    doSort(files);

    {
        QList<QUrl> fileUrls;
        QMap<QUrl, DFMLocalFileInfoPointer> fileMaps;
        for (auto itemInfo : files) {
            fileUrls.append(itemInfo->url());
            fileMaps.insert(itemInfo->url(), itemInfo);
        }

        model()->beginResetModel();
        fileList = fileUrls;
        fileMap = fileMaps;
        model()->endResetModel();
    }
}

DSB_D_END_NAMESPACE
