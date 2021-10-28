/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "recentfileinfo.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"
#include "private/dabstractfileinfo_p.h"
#include "app/define.h"
#include "singleton.h"
#include "controllers/pathmanager.h"
#include "shutil/mimetypedisplaymanager.h"
#include <QXmlStreamReader>
#include <QFile>

namespace FileSortFunction
{
COMPARE_FUN_DEFINE(readDateTime, LastReadTime, RecentFileInfo)
COMPARE_FUN_DEFINE(toLocalFile, RecentFilePath, RecentFileInfo)
}

RecentFileInfo::RecentFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
    QMutexLocker lk(&m_mutex);
    if (url.path() != "/") {
        setProxy(DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(url.path())));
    }
    updateInfo();
    checkMountFile();
}

RecentFileInfo::~RecentFileInfo()
{
    QMutexLocker lk(&m_mutex);
}

bool RecentFileInfo::makeAbsolute()
{
    return true;
}

bool RecentFileInfo::exists() const
{
    if (fileUrl() == DUrl(RECENT_ROOT))
        return true;

    return DAbstractFileInfo::exists();
}

bool RecentFileInfo::isDir() const
{
    return fileUrl() == DUrl(RECENT_ROOT);
}

bool RecentFileInfo::isReadable() const
{
    return permissions().testFlag(QFile::Permission::ReadUser);
}

bool RecentFileInfo::isWritable() const
{
    if (isGvfsMountFile()) {
        if (m_isWriteAble == -1)
            const_cast<RecentFileInfo*>(this)->m_isWriteAble =
                permissions().testFlag(QFile::Permission::WriteUser);
        return m_isWriteAble > 0;
    }
    return permissions().testFlag(QFile::Permission::WriteUser);
}

bool RecentFileInfo::canIteratorDir() const
{
    return true;
}

bool RecentFileInfo::canDrop() const
{
    return false;
}

bool RecentFileInfo::canRedirectionFileUrl() const
{
    return d_ptr->proxy;
}

bool RecentFileInfo::canRename() const
{
    return false;
}

DUrl RecentFileInfo::redirectedFileUrl() const
{
    return d_ptr->proxy ? d_ptr->proxy->fileUrl() : fileUrl();
}

DUrl RecentFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    Q_D(const DAbstractFileInfo);

    return d->proxy ? d->proxy->getUrlByNewFileName(fileName) :
                      DAbstractFileInfo::getUrlByNewFileName(fileName);
}

QFileDevice::Permissions RecentFileInfo::permissions() const
{
    if (fileUrl() == DUrl(RECENT_ROOT)) {
        return QFileDevice::ReadGroup | QFileDevice::ReadOwner | QFileDevice::ReadOther;
    }

    QFileDevice::Permissions p = DAbstractFileInfo::permissions();

    return p;
}

QVector<MenuAction> RecentFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_D(const DAbstractFileInfo);

    QVector<MenuAction> actions;

    if (type == SpaceArea) {
        actions << MenuAction::DisplayAs;
        actions << MenuAction::SortBy;
        actions << MenuAction::SelectAll;

        return actions;
    }

    actions = d->proxy->menuActionList(type);

    qDebug() << actions;

    actions.insert(1, MenuAction::OpenFileLocation);
    int indexOfDeleteMenuAction = actions.indexOf(MenuAction::Delete);
    if (indexOfDeleteMenuAction == -1) {
        int indexOfCompleteDelectionMenuAction = actions.indexOf(MenuAction::CompleteDeletion);
        if (indexOfCompleteDelectionMenuAction != -1) {
            actions.insert(indexOfCompleteDelectionMenuAction, MenuAction::RemoveFromRecent);
        }
    } else {
        actions.insert(indexOfDeleteMenuAction, MenuAction::RemoveFromRecent);
    }
    actions.removeOne(MenuAction::Cut);
    actions.removeOne(MenuAction::Rename);
    actions.removeOne(MenuAction::Delete);
    actions.removeOne(MenuAction::CompleteDeletion);
    actions.removeOne(MenuAction::DecompressHere);
    actions.removeOne(MenuAction::StageFileForBurning);

    return actions;
}

QSet<MenuAction> RecentFileInfo::disableMenuActionList() const
{
    Q_D(const DAbstractFileInfo);

    if (fileUrl() == DUrl(RECENT_ROOT))
        return {};

    return d->proxy->disableMenuActionList();
}

QList<int> RecentFileInfo::userColumnRoles() const
{
    static QList<int> userColumnRoles = QList<int>() << DFileSystemModel::FileDisplayNameRole
                                                     << DFileSystemModel::FileUserRole + 1
                                                     << DFileSystemModel::FileLastReadRole
                                                     << DFileSystemModel::FileSizeRole
                                                     << DFileSystemModel::FileMimeTypeRole;

    return userColumnRoles;
}

QVariant RecentFileInfo::userColumnData(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileLastReadRole) {
        return m_lastReadTimeStr;
    }

    if (userColumnRole == DFileSystemModel::FileUserRole + 1) {
        return toLocalFile();
    }

    return DAbstractFileInfo::userColumnData(userColumnRole);
}


QVariant RecentFileInfo::userColumnDisplayName(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1){
        return qApp->translate("DFileSystemModel",  "Path");
    }

    return DAbstractFileInfo::userColumnDisplayName(userColumnRole);
}

MenuAction RecentFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1) {
        return MenuAction::AbsolutePath;
    }

    return DAbstractFileInfo::menuActionByColumnRole(userColumnRole);
}


int RecentFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    if (userColumnRole == DFileSystemModel::FileNameRole)
        return -1;

    return DAbstractFileInfo::userColumnWidth(userColumnRole, fontMetrics);
}

DUrl RecentFileInfo::mimeDataUrl() const
{
    return fileUrl();
}

DUrl RecentFileInfo::parentUrl() const
{
    return DUrl(RECENT_ROOT);
}

DAbstractFileInfo::CompareFunction RecentFileInfo::compareFunByColumn(int columnRole) const
{
    // see RecentFileInfo::userColumnRoles for role function
    if (columnRole == DFileSystemModel::FileLastReadRole) {
        return FileSortFunction::compareFileListByLastReadTime;
    } else if (columnRole == DFileSystemModel::FileUserRole + 1) {
        return FileSortFunction::compareFileListByRecentFilePath;
    } else {
        return DAbstractFileInfo::compareFunByColumn(columnRole);
    }
}

QString RecentFileInfo::toLocalFile() const
{
    return fileUrl().path();
}

QString RecentFileInfo::subtitleForEmptyFloder() const
{
    return QObject::tr("Folder is empty");
}

DUrl RecentFileInfo::goToUrlWhenDeleted() const
{
    return DUrl::fromLocalFile(QDir::homePath());
}

QString RecentFileInfo::fileDisplayName() const
{
    Q_D(const DAbstractFileInfo);

    if (fileUrl() == DUrl(RECENT_ROOT))
        return systemPathManager->getSystemPathDisplayName("Recent");

    return d->proxy->fileDisplayName();
}

QString RecentFileInfo::mimeTypeDisplayName() const
{
    Q_D(const DAbstractFileInfo);

    if (fileUrl() == DUrl(RECENT_ROOT))
        return mimeTypeDisplayManager->displayName("");

    return d->proxy->mimeTypeDisplayName();
}

bool RecentFileInfo::isVirtualEntry() const
{
    Q_D(const DAbstractFileInfo);

    if (fileUrl() == DUrl(RECENT_ROOT))
        return true;

    return d->proxy->isVirtualEntry();
}

const QDateTime RecentFileInfo::getReadTime() const
{
    return m_lastReadTime;
}

void RecentFileInfo::updateReadTime(const QDateTime &date)
{
    setReadDateTime(date.toString(Qt::ISODate));
}

void RecentFileInfo::updateInfo()
{
    QFile file(QDir::homePath() + "/.local/share/recently-used.xbel");

    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);

        while (!reader.atEnd()) {
            if (!reader.readNextStartElement() ||
                 reader.name() != "bookmark") {
                continue;
            }

            const QStringRef &location = reader.attributes().value("href");
            const QStringRef &dateTime = reader.attributes().value("modified");

            if (!location.isEmpty()) {
                DUrl findUrl = DUrl(location.toString());

                if (findUrl.toLocalFile() == fileUrl().path()) {
                    setReadDateTime(dateTime.toString());
                    break;
                }
            }
        }
    }
}

void RecentFileInfo::setReadDateTime(const QString &time)
{
    m_lastReadTime = QDateTime::fromString(time, Qt::ISODate).toLocalTime();
    m_lastReadTimeStr = m_lastReadTime.toString(dateTimeFormat());
}

QDateTime RecentFileInfo::readDateTime() const
{
    return m_lastReadTime;
}
