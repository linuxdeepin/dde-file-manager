/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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

#include "dfmplugin_bookmark_global.h"

#include "dfm-base/interfaces/abstractfilewatcher.h"

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
    int index = -1;

    QString udisksDBusPath;
    QString udisksMountPoint;
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

    void addBookMarkItem(const QUrl &url, const QString &bookmarkName) const;
    void addBookMarkItemsFromConfig();

    static void contextMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static void renameCallBack(quint64 windowId, const QUrl &url, const QString &name);
    static void cdBookMarkUrlCallBack(quint64 windowId, const QUrl &url);

    static QString bookMarkActionCreatedCallBack(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected);
    static void bookMarkActionClickedCallBack(bool isNormal, const QUrl &currentUrl, const QUrl &focusFile, const QList<QUrl> &selected);

    void fileRenamed(const QUrl &oldUrl, const QUrl &newUrl);

    void addSchemeOfBookMarkDisabled(const QString &scheme);
    QMap<QUrl, BookmarkData> getBookMarkDataMap() const;
    int defaultItemIndex(const QUrl &url);
    QMap<QUrl, QPair<QString, int>> defaultNames() const;
    bool handleItemSort(const QUrl &a, const QUrl &b);

private:
    explicit BookMarkManager(QObject *parent = nullptr);
    void update(const QVariant &value);
    void removeAllBookMarkSidebarItems();
    void bookMarkRename(const QUrl &url, const QString &newName);
    int showRemoveBookMarkDialog(quint64 winId);
    void getMountInfo(const QUrl &url, QString &mountPoint, QString &localUrl);
    QSet<QString> getBookMarkDisabledSchemes();
    void sortItemsByOrder(const QList<QUrl> &order);
    static bool isDublicated(const QString &name);

private slots:
    void onFileEdited(const QString &group, const QString &key, const QVariant &value);

private:
    static /*mutable*/ QMap<QUrl, BookmarkData> bookmarkDataMap;
    QSet<QString> bookMarkDisabledSchemes;
    static QMap<QUrl, QPair<QString, int>> defaultItemNames;
    static const QList<QPair<QString, QString>> defNames;
};

}

#endif   // BOOKMARKMANAGER_H
