// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include "dabstractfilecontroller.h"

#include "models/bookmark.h"

#include <deque>

#include <QDir>

class DAbstractFileInfo;
class DBookmarkItem;

struct BookmarkData {
    DUrl m_url = DUrl();
    QDateTime m_created = QDateTime();
    QDateTime m_lastModified = QDateTime();
    QString mountPoint = QString();
    QString locateUrl = QString();
    QString udisksDBusPath = QString();
    QString udisksMountPoint = QString();
};

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
    void refreshBookmark();
    const DUrlList getBookmarkUrls();
    bool isEqualBookmarkData(const QMap<QString, QVariant> arg1, const QMap<QString, QVariant> &arg2) const;
    void mergeList(const QVariantList &oldList, const QVariantList &groupPolicyList, QVariantList &all) const;

    void initData();

private:
    BookMarkPointer findBookmark(const DUrl &url) const;
    BookmarkData findBookmarkData(const DUrl &url) const;
    mutable QMap<DUrl, BookMarkPointer> m_bookmarks;
    mutable QMap<DUrl, BookmarkData> m_bookmarkDataMap;

    void update(const QVariant &value);
    void onFileEdited(const QString &group, const QString &key, const QVariant &value);
    void variantToBookmarkData(const QMap<QString, QVariant> &item, BookmarkData &data);
//    std::deque<QExplicitlySharedDataPointer<BookMark>> m_tagBookmarks{};
};

#endif // BOOKMARKMANAGER_H
