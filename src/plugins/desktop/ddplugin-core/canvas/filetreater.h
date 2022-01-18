/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#ifndef FILETREATER_H
#define FILETREATER_H

#include "dfm-base/file/local/localfileinfo.h"
#include "dfm_desktop_service_global.h"

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

DSB_D_BEGIN_NAMESPACE

class FileTreaterPrivate;
class FileTreater : public QObject
{
    Q_OBJECT
public:

    // tod(wangcl) some need del
    enum Roles {
        kFileIconRole = Qt::DecorationRole,
//        kFilePathRole = Qt::UserRole + 1,
        kFileNameRole = Qt::UserRole + 2,
        kFileSizeRole = Qt::UserRole + 3,
        kFileMimeTypeRole = Qt::UserRole + 4,
//        kFileOwnerRole = Qt::UserRole + 5,
        kFileLastModifiedRole = Qt::UserRole + 6,
//        kFileLastReadRole = Qt::UserRole + 7,
//        kFileCreatedRole = Qt::UserRole + 8,
        kFileDisplayNameRole = Qt::UserRole + 9,
//        kFilePinyinName = Qt::UserRole + 10,
        kExtraProperties = Qt::UserRole + 11,
//        kFileBaseNameRole = Qt::UserRole + 12,
        kFileSuffixRole = Qt::UserRole + 13,
        kFileNameOfRenameRole = Qt::UserRole + 14,
        kFileBaseNameOfRenameRole = Qt::UserRole + 15,
        kFileSuffixOfRenameRole = Qt::UserRole + 16,
//        kFileSizeInKiloByteRole = Qt::UserRole + 17,
//        kFileLastModifiedDateTimeRole = Qt::UserRole + 18,
//        kFileIconModelToolTipRole = Qt::UserRole + 19,
//        kFileLastReadDateTimeRole = Qt::UserRole + 20,
//        kFileCreatedDateTimeRole = Qt::UserRole + 21,
//        kFileUserRole = Qt::UserRole + 99,
        kUnknowRole = Qt::UserRole + 999
    };

    // todo: move to menu manager
    enum MenuAction {
        kName,
        kSize,
        kType,
        kLastModifiedDate
    };

    static FileTreater *instance();
    void init();
    DFMLocalFileInfoPointer fileInfo(const QString &url);
    DFMLocalFileInfoPointer fileInfo(int index);

    int indexOfChild(AbstractFileInfoPointer info);
    const QList<QUrl> &getFiles() const;
    int fileCount() const;

    QUrl desktopUrl() const;
    bool canRefresh() const;
    void refresh();
    bool isRefreshed() const;

    bool enableSort() const;
    void setEnabledSort(const bool enabledSort);
    bool sort();

    Qt::SortOrder sortOrder() const;
    void setSortOrder(const Qt::SortOrder order);

    int sortRole() const;
    void setSortRole(const dfmbase::AbstractFileInfo::SortKey role, const Qt::SortOrder order = Qt::AscendingOrder);

    bool whetherShowHiddenFiles() const;
    void setWhetherShowHiddenFiles(const bool isShow);

signals:
    void fileCreated(const QUrl &url);
    void fileDeleted(const QUrl &url);
    void fileRenamed(const QUrl &oldUrl, const QUrl &newUrl);

    void fileRefreshed();
    void fileSorted();
    void enableSortChanged(bool enableSort);

protected:
    explicit FileTreater(QObject *parent = nullptr);
    ~FileTreater();

private:
    QSharedPointer<FileTreaterPrivate> d;
};

#define FileTreaterCt DSB_D_NAMESPACE::FileTreater::instance()

DSB_D_END_NAMESPACE
#endif   // FILETREATER_H
