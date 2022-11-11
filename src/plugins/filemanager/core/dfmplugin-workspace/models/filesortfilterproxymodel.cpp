/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#include "filesortfilterproxymodel.h"
#include "fileviewmodel.h"
#include "utils/workspacehelper.h"
#include "views/fileview.h"

#include "base/application/settings.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <QTimer>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileSortFilterProxyModel::FileSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    resetFilter();
}

FileSortFilterProxyModel::~FileSortFilterProxyModel()
{
}

QVariant FileSortFilterProxyModel::headerData(int column, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        int column_role = getRoleByColumn(column);
        return roleDisplayString(column_role);
    }

    return QVariant();
}

Qt::ItemFlags FileSortFilterProxyModel::flags(const QModelIndex &index) const
{
    // single select mode will check index enabled when remove row
    if (!index.isValid())
        return Qt::ItemFlag::ItemIsEnabled;

    Qt::ItemFlags flags = QSortFilterProxyModel::flags(index);

    const AbstractFileInfoPointer &info = itemFileInfo(index);

    if (info) {
        if (!passNameFilters(info))
            flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }

    if (readOnly)
        flags &= ~(Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);

    return flags;
}

Qt::DropActions FileSortFilterProxyModel::supportedDragActions() const
{
    const QModelIndex &rootIndex = viewModel()->findRootIndex(rootUrl);
    const AbstractFileInfoPointer info = viewModel()->fileInfo(rootIndex);

    if (info)
        return info->supportedDragActions();

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions FileSortFilterProxyModel::supportedDropActions() const
{
    const QModelIndex &rootIndex = viewModel()->findRootIndex(rootUrl);
    const AbstractFileInfoPointer info = viewModel()->fileInfo(rootIndex);

    if (info)
        return info->supportedDropActions();

    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

QModelIndex FileSortFilterProxyModel::setRootUrl(const QUrl &url)
{
    rootUrl = url;

    workStoped = false;
    const QModelIndex &rootIndex = viewModel()->setRootUrl(url);
    resetFilter();

    return mapFromSource(rootIndex);
}

QUrl FileSortFilterProxyModel::currentRootUrl() const
{
    return rootUrl;
}

void FileSortFilterProxyModel::clear()
{
    viewModel()->clear(rootUrl);
}

void FileSortFilterProxyModel::update()
{
    viewModel()->update(rootUrl);
}

AbstractFileInfoPointer FileSortFilterProxyModel::itemFileInfo(const QModelIndex &index) const
{
    const QModelIndex &sourceIndex = mapToSource(index);
    return viewModel()->fileInfo(sourceIndex);
}

QModelIndex FileSortFilterProxyModel::getIndexByUrl(const QUrl &url) const
{
    const QModelIndex &sourceIndex = viewModel()->findChildIndex(url);
    const QModelIndex &proxyIndex = mapFromSource(sourceIndex);

    if (proxyIndex.isValid())
        return proxyIndex;

    return QModelIndex();
}

QUrl FileSortFilterProxyModel::getUrlByIndex(const QModelIndex &index) const
{
    const QModelIndex &sourceIndex = mapToSource(index);
    return sourceIndex.data(kItemUrlRole).toUrl();
}

QList<QUrl> FileSortFilterProxyModel::getCurrentDirFileUrls() const
{
    return viewModel()->getChildrenUrls(rootUrl);
}

int FileSortFilterProxyModel::getColumnWidth(const int &column) const
{
    const ItemRoles role = getRoleByColumn(column);

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();
    int colWidth = state.value(QString::number(role), -1).toInt();
    if (colWidth > 0) {
        return colWidth;
    }

    return 120;
}

ItemRoles FileSortFilterProxyModel::getRoleByColumn(const int &column) const
{
    QList<ItemRoles> columnRoleList = viewModel()->getColumnRoles(rootUrl);

    if (columnRoleList.length() > column)
        return columnRoleList.at(column);

    return kItemFileDisplayNameRole;
}

int FileSortFilterProxyModel::getColumnByRole(const ItemRoles role) const
{
    QList<ItemRoles> columnRoleList = viewModel()->getColumnRoles(rootUrl);
    return columnRoleList.indexOf(role) < 0 ? 0 : columnRoleList.indexOf(role);
}

QDir::Filters FileSortFilterProxyModel::getFilters() const
{
    return filters;
}

QStringList FileSortFilterProxyModel::getNameFilters() const
{
    return nameFilters;
}

void FileSortFilterProxyModel::setFilters(const QDir::Filters &filters)
{
    this->filters = filters;

    invalidateFilter();
}

void FileSortFilterProxyModel::setNameFilters(const QStringList &nameFilters)
{
    if (this->nameFilters == nameFilters) {
        return;
    }

    nameFiltersMatchResultMap.clear();
    this->nameFilters = nameFilters;

    invalidateFilter();
}

void FileSortFilterProxyModel::setFilterData(const QVariant &data)
{
    filterData = data;
    invalidateFilter();
}

void FileSortFilterProxyModel::setFilterCallBack(const FileViewFilterCallback callback)
{
    filterCallback = callback;
    invalidateFilter();
}

void FileSortFilterProxyModel::resetFilter()
{
    filterData = QVariant();
    filterCallback = nullptr;
    filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    bool isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();
    if (isShowedHiddenFiles) {
        filters |= QDir::Hidden;
    } else {
        filters &= ~QDir::Hidden;
    }
    invalidateFilter();
}

void FileSortFilterProxyModel::toggleHiddenFiles()
{
    filters = ~(filters ^ QDir::Filter(~QDir::Hidden));
    setFilters(filters);
}

void FileSortFilterProxyModel::setReadOnly(const bool readOnly)
{
    this->readOnly = readOnly;
}

void FileSortFilterProxyModel::stopWork()
{
    workStoped = true;
    viewModel()->stopTraversWork(rootUrl);
}

void FileSortFilterProxyModel::setActive(const QModelIndex &index, bool enable)
{
    const QModelIndex &sourceIndex = mapToSource(index);
    viewModel()->setIndexActive(sourceIndex, enable);
}

ModelState FileSortFilterProxyModel::currentState() const
{
    return state;
}

void FileSortFilterProxyModel::initMixDirAndFile()
{
    isNotMixDirAndFile = !Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
}

void FileSortFilterProxyModel::onChildrenUpdate(const QUrl &url)
{
    if (UniversalUtils::urlEquals(url, rootUrl))
        Q_EMIT modelChildrenUpdated();
}

void FileSortFilterProxyModel::onTraverPrehandle(const QUrl &url, const QModelIndex &index)
{
    if (UniversalUtils::urlEquals(url, rootUrl)) {
        auto prehandler = WorkspaceHelper::instance()->viewRoutePrehandler(url.scheme());
        if (prehandler) {
            isPrehandling = true;
            quint64 winId = FileManagerWindowsManager::instance().findWindowId(dynamic_cast<FileView *>(parent()));
            QPointer<FileViewModel> guard(viewModel());
            prehandler(winId, url, [guard, index, this]() {
                if (guard)
                    guard->doFetchMore(index);

                this->isPrehandling = false;
            });
        }
    }
}

void FileSortFilterProxyModel::onStateChanged(const QUrl &url, ModelState state)
{
    if (UniversalUtils::urlEquals(url, rootUrl)) {
        if (state == this->state || isPrehandling)
            return;

        this->state = state;

        if (workStoped)
            return;

        Q_EMIT stateChanged();
    }
}

void FileSortFilterProxyModel::onSelectAndEditFile(const QUrl &rootUrl, const QUrl &url)
{
    if (UniversalUtils::urlEquals(rootUrl, this->rootUrl)) {
        quint64 winId = WorkspaceHelper::instance()->windowId(dynamic_cast<QWidget *>(parent()));
        if (WorkspaceHelper::kSelectionAndRenameFile.contains(winId)) {
            if (WorkspaceHelper::kSelectionAndRenameFile[winId].first == rootUrl) {
                WorkspaceHelper::kSelectionAndRenameFile[winId] = qMakePair(QUrl(), QUrl());

                QTimer::singleShot(100, this, [=] {
                    emit selectAndEditFile(url);
                });
            }
        }
    }
}

bool FileSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (!left.isValid())
        return false;
    if (!right.isValid())
        return false;

    const QModelIndex &leftParent = left.parent();
    if (!leftParent.isValid() || !UniversalUtils::urlEquals(viewModel()->fileInfo(QModelIndex(), leftParent)->url(), rootUrl))
        return false;
    const QModelIndex &rightParent = right.parent();
    if (!rightParent.isValid() || !UniversalUtils::urlEquals(viewModel()->fileInfo(QModelIndex(), rightParent)->url(), rootUrl))
        return false;

    const AbstractFileInfoPointer &leftInfo = viewModel()->fileInfo(leftParent, left);
    const AbstractFileInfoPointer &rightInfo = viewModel()->fileInfo(rightParent, right);

    if (!leftInfo)
        return false;
    if (!rightInfo)
        return false;

    // The folder is fixed in the front position
    if (isNotMixDirAndFile) {
        if (leftInfo->isDir()) {
            if (!rightInfo->isDir())
                return sortOrder() == Qt::AscendingOrder;
        } else {
            if (rightInfo->isDir())
                return sortOrder() == Qt::DescendingOrder;
        }
    }

    QVariant leftData = viewModel()->data(left, sortRole());
    QVariant rightData = viewModel()->data(right, sortRole());

    // When the selected sort attribute value is the same, sort by file name
    if (leftData == rightData) {
        QString leftName = viewModel()->data(left, kItemFileDisplayNameRole).toString();
        QString rightName = viewModel()->data(right, kItemFileDisplayNameRole).toString();
        return FileUtils::compareString(leftName, rightName, sortOrder());
    }

    switch (sortRole()) {
    case kItemFileDisplayNameRole:
    case kItemFileLastModifiedRole:
    case kItemFileMimeTypeRole:
        return FileUtils::compareString(leftData.toString(), rightData.toString(), sortOrder()) == (sortOrder() == Qt::AscendingOrder);
    case kItemFileSizeRole:
        if (isNotMixDirAndFile) {
            if (leftInfo->isDir()) {
                int leftCount = qSharedPointerDynamicCast<AbstractFileInfo>(leftInfo)->countChildFile();
                int rightCount = qSharedPointerDynamicCast<AbstractFileInfo>(rightInfo)->countChildFile();
                return leftCount < rightCount;
            } else {
                return leftInfo->size() < rightInfo->size();
            }
        } else {
            qint64 sizel = leftInfo->isDir() && rightInfo->isDir() ? qSharedPointerDynamicCast<AbstractFileInfo>(leftInfo)->countChildFile() : leftInfo->isDir() ? 0 : leftInfo->size();
            qint64 sizer = leftInfo->isDir() && rightInfo->isDir() ? qSharedPointerDynamicCast<AbstractFileInfo>(rightInfo)->countChildFile() : rightInfo->isDir() ? 0 : rightInfo->size();
            return sizel < sizer;
        }
    default:
        return FileUtils::compareString(leftData.toString(), rightData.toString(), sortOrder()) == (sortOrder() == Qt::AscendingOrder);
    }
}

bool FileSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex rowIndex = viewModel()->index(sourceRow, 0, sourceParent);

    if (!rowIndex.isValid())
        return false;

    // root index should not be filtered
    if (!sourceParent.isValid())
        return true;

    const AbstractFileInfoPointer &fileInfo = viewModel()->fileInfo(rowIndex);

    return passFileFilters(fileInfo);
}

bool FileSortFilterProxyModel::passFileFilters(const AbstractFileInfoPointer &fileInfo) const
{
    if (!fileInfo)
        return false;

    if (filterCallback && !filterCallback(fileInfo.data(), filterData))
        return false;

    if (filters == QDir::NoFilter)
        return true;

    if (!(filters & (QDir::Dirs | QDir::AllDirs)) && fileInfo->isDir())
        return false;

    if (!(filters & QDir::Files) && fileInfo->isFile())
        return false;

    if ((filters & QDir::NoSymLinks) && fileInfo->isSymLink())
        return false;

    if (!(filters & QDir::Hidden) && fileInfo->isHidden())
        return false;

    if ((filters & QDir::Readable) && !fileInfo->isReadable())
        return false;

    if ((filters & QDir::Writable) && !fileInfo->isWritable())
        return false;

    if ((filters & QDir::Executable) && !fileInfo->isExecutable())
        return false;

    return true;
}

bool FileSortFilterProxyModel::passNameFilters(const AbstractFileInfoPointer &info) const
{
    if (nameFilters.isEmpty())
        return true;

    if (!info)
        return true;

    const QUrl fileUrl = info->url();
    if (nameFiltersMatchResultMap.contains(fileUrl))
        return nameFiltersMatchResultMap.value(fileUrl, false);

    // Check the name regularexpression filters
    if (!(info->isDir() && (filters & QDir::Dirs))) {
        const Qt::CaseSensitivity caseSensitive = (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
        const QString &fileName = info->fileName();
        QRegExp re("", caseSensitive, QRegExp::Wildcard);

        for (int i = 0; i < nameFilters.size(); ++i) {
            re.setPattern(nameFilters.at(i));
            if (re.exactMatch(fileName)) {
                nameFiltersMatchResultMap[fileUrl] = true;
                return true;
            }
        }

        nameFiltersMatchResultMap[fileUrl] = false;
        return false;
    }

    nameFiltersMatchResultMap[fileUrl] = true;
    return true;
}

FileViewModel *FileSortFilterProxyModel::viewModel() const
{
    return qobject_cast<FileViewModel *>(sourceModel());
}

QString FileSortFilterProxyModel::roleDisplayString(int role) const
{
    QString displayName;

    if (WorkspaceEventSequence::instance()->doFetchCustomRoleDiaplayName(rootUrl, static_cast<ItemRoles>(role), &displayName))
        return displayName;

    switch (role) {
    case kItemFileDisplayNameRole:
        return tr("Name");
    case kItemFileLastModifiedRole:
        return tr("Time modified");
    case kItemFileSizeRole:
        return tr("Size");
    case kItemFileMimeTypeRole:
        return tr("Type");
    default:
        return QString();
    }
}

QList<ItemRoles> FileSortFilterProxyModel::getColumnRoles() const
{
    return viewModel()->getColumnRoles(rootUrl);
}
