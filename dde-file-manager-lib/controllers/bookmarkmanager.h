/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include "dabstractfilecontroller.h"

#include "models/bookmark.h"

#include <deque>

#include <QDir>

class DAbstractFileInfo;
class DBookmarkItem;

class BookMarkManager : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit BookMarkManager(QObject *parent = nullptr);
    ~BookMarkManager() override;

    bool checkExist(const DUrl &url);

    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;
    bool touch(const QSharedPointer<DFMTouchFileEvent> &event) const override;
    bool setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const override;

    bool removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const override;

    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    bool onFileRenamed(const DUrl &from, const DUrl &to);

private:
    BookMarkPointer findBookmark(const DUrl &url) const;
    mutable QMap<DUrl, BookMarkPointer> m_bookmarks;

    void update(const QVariant &value);
    void onFileEdited(const QString &group, const QString &key, const QVariant &value);
//    std::deque<QExplicitlySharedDataPointer<BookMark>> m_tagBookmarks{};
};

#endif // BOOKMARKMANAGER_H
