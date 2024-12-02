// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include "dfmplugin_bookmark_global.h"

#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <QObject>
#include <QUrl>
#include <QDateTime>
#include <QMap>
#include <QSet>

namespace dfmplugin_bookmark {

struct BookmarkData
{
    QDateTime created;
    QDateTime lastModified;
    QString locateUrl;
    QString deviceUrl;
    QString name;
    QUrl url;
    bool isDefaultItem = false;
    int index = -1;
    QVariantMap sidebarProperties;

    void resetData(const QVariantMap &map);
    QVariantMap serialize();
};
class BookMarkWatcherController;
class BookMarkManager : public QObject
{
    Q_OBJECT
    friend class BookMarkEventReceiver;

public:
    static BookMarkManager *instance();

    bool addBookMark(const QList<QUrl> &urls);
    bool removeBookMark(const QUrl &url);

    void addBookMarkItem(const QUrl &url, const QString &bookmarkName, bool isDefaultItem = false) const;
    void addQuickAccessItemsFromConfig();

    void fileRenamed(const QUrl &oldUrl, const QUrl &newUrl);

    void addSchemeOfBookMarkDisabled(const QString &scheme);
    QMap<QUrl, BookmarkData> getBookMarkDataMap() const;
    void initData();
    bool isItemDuplicated(const BookmarkData &data);
    bool bookMarkRename(const QUrl &url, const QString &newName);
    QSet<QString> getBookmarkDisabledSchemes();
    int showRemoveBookMarkDialog(quint64 winId);

private:
    explicit BookMarkManager(QObject *parent = nullptr);
    void update(const QVariant &value);
    void removeAllBookMarkSidebarItems();

    void getMountInfo(const QUrl &url, QString &mountPoint, QString &localUrl);
    void saveSortedItemsToConfigFile(const QList<QUrl> &order);
    void saveQuickAccessToSortedItems(const QVariantList &list);

    void addQuickAccessDataFromConfig(const QVariantList &dataList = QVariantList());

    void removeBookmarkFromDConfig(const QUrl &url);
    void addBookmarkToDConfig(const QVariantMap &data);
    void renameBookmarkToDConfig(const QString &oldName, const QString &newName);
    void updateBookmarkUrlToDconfig(const QUrl &oldUrl, const QUrl &newUrl);

private slots:
    void onFileEdited(const QString &group, const QString &key, const QVariant &value);

private:
    QMap<QUrl, BookmarkData> quickAccessDataMap = {};
    QSet<QString> bookmarkDisabledSchemes;
    QList<QUrl> sortedUrls;
};

}

#endif   // BOOKMARKMANAGER_H
