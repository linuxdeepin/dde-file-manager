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

#include "basemanager.h"
#include "dabstractfilecontroller.h"

#include "models/bookmark.h"

#include <deque>

#include <QDir>

class DAbstractFileInfo;
class DBookmarkItem;

class BookMarkManager : public DAbstractFileController, public BaseManager
{
    Q_OBJECT
public:
    explicit BookMarkManager(QObject *parent = 0);
    ~BookMarkManager();

    void load();
    void save() const;
    void moveBookmark(int from, int to);
    int getBookmarkIndex(const DUrl &url);
    QList<BookMarkPointer> getBookmarks();

    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const Q_DECL_OVERRIDE;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    bool touch(const QSharedPointer<DFMTouchFileEvent> &event) const Q_DECL_OVERRIDE;

    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const Q_DECL_OVERRIDE;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const Q_DECL_OVERRIDE;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const Q_DECL_OVERRIDE;

    ///###: tag protocol. Maybe will be used later.
//    void appendBookmark(QExplicitlySharedDataPointer<BookMark> bookmarkPointer)noexcept;
//    void appendTagBookmark(QExplicitlySharedDataPointer<BookMark> bookmark) noexcept;
//    void clearTagBookmark();


    static QString cachePath();
private:
    void loadJson(const QJsonObject &json);
    void writeJson(QJsonObject &json) const;
    BookMarkPointer findBookmark(const DUrl &url) const;
    QList<BookMarkPointer> m_bookmarks;

//    std::deque<QExplicitlySharedDataPointer<BookMark>> m_tagBookmarks{};

public slots:
    Q_DECL_DEPRECATED void removeBookmark(BookMarkPointer bookmark);
    Q_DECL_DEPRECATED void renameBookmark(BookMarkPointer bookmark, const QString &newname);

    void reLoad();
};

#endif // BOOKMARKMANAGER_H
