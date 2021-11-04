/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "bookmarkmanager.h"
#include "dfileservices.h"
#include "dfmevent.h"
#include "dabstractfilewatcher.h"
#include "private/dabstractfilewatcher_p.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

#include "app/define.h"

#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>
#include <QUrlQuery>
#include <QtConcurrent>

#include <stdlib.h>

#include "shutil/fileutils.h"
#include "deviceinfo/udisklistener.h"

class BookMarkFileWatcher;
class BookMarkFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit BookMarkFileWatcherPrivate(DAbstractFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override
    {
        started = true;

        return true;
    }

    bool stop() override
    {
        started = false;

        return true;
    }
};

class BookMarkFileWatcher : public DAbstractFileWatcher
{
public:
    explicit BookMarkFileWatcher(const DUrl &url, QObject *parent = nullptr)
        : DAbstractFileWatcher(*new BookMarkFileWatcherPrivate(this), url, parent)
    {

    }
};

BookMarkManager::BookMarkManager(QObject *parent)
    : DAbstractFileController(parent)
{
    fileService->setFileUrlHandler(BOOKMARK_SCHEME, "", this);

    connect(DFMApplication::genericSetting(), &DFMSettings::valueEdited, this, &BookMarkManager::onFileEdited);
    connect(DFileService::instance(), &DFileService::fileRenamed, this, &BookMarkManager::onFileRenamed);
}

BookMarkManager::~BookMarkManager()
{

}

/*!
 * \brief Check if a bookmark is exist.
 * \param url a bookmark url
 *
 * FIXME: A workaround for the worng implement of BookMark::exists().
 * We should remove this function or make it private once we correct
 * BookMark::exists() 's behavior.
 *
 * \return exist or not
 */
bool BookMarkManager::checkExist(const DUrl &url)
{
    return m_bookmarkDataMap.contains(url.bookmarkTargetUrl());
//    BookMarkPointer p = m_bookmarks.value(url.bookmarkTargetUrl());
//    return p;
}

bool BookMarkManager::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    DUrl from = event->fromUrl();
    DUrl new_from = from.bookmarkTargetUrl();
    DUrl to = event->toUrl();

    BookmarkData data = findBookmarkData(event->fromUrl());

    if (!data.m_url.isValid()) {
        return false;
    }

    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();

    for (int i = 0; i < list.count(); ++i) {
        QVariantMap map = list.at(i).toMap();

        if (map.value("name").toString() == data.m_url.bookmarkName()) {
            map["name"] = event->toUrl().bookmarkName();
            list[i] = map;

            DFMApplication::genericSetting()->setValue("BookMark", "Items", list);

            data.m_url = event->toUrl();
            data.m_lastModified = QDateTime::currentDateTime();
            m_bookmarkDataMap[event->toUrl().bookmarkTargetUrl()] = data;

            BookMarkPointer item = findBookmark(event->fromUrl());
            if (item) {
                BookMark *new_item = new BookMark(event->toUrl());
                QUrlQuery query(event->toUrl());

                new_item->m_created = data.m_created;
                new_item->m_lastModified = data.m_lastModified;
                new_item->mountPoint = data.mountPoint;//query.queryItemValue("mount_point");
                new_item->locateUrl = map.value("locateUrl").toString();//query.queryItemValue("locate_url");

                m_bookmarks[event->toUrl().bookmarkTargetUrl()] = new_item;
            }

            break;
        }
    }

    DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileMoved, event->fromUrl(), event->toUrl());

    return true;
}

bool BookMarkManager::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();

    for (const DUrl &url : event->urlList()) {

        if (!m_bookmarkDataMap.contains(url.bookmarkTargetUrl()))
            continue;

        m_bookmarks.remove(url.bookmarkTargetUrl());
        const BookmarkData &data = m_bookmarkDataMap.take(url.bookmarkTargetUrl());

        for (int i = 0; i < list.count(); ++i) {
            const QVariantMap &map = list.at(i).toMap();

            if (map.value("name").toString() == data.m_url.bookmarkName()) {
                list.removeAt(i);
                break;
            }
        }

        DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileDeleted, data.m_url);
    }

    DFMApplication::genericSetting()->setValue("BookMark", "Items", list);

    return true;
}

bool BookMarkManager::touch(const QSharedPointer<DFMTouchFileEvent> &event) const
{
    //与其他书签储存逻辑保持一致，去掉url中的Query字符串
    DUrl newUrl = event->url();
    newUrl.setQuery("");
    QUrlQuery query(event->url());

    BookmarkData bookmarkData;
    bookmarkData.m_created = QDateTime::currentDateTime();
    bookmarkData.m_lastModified = bookmarkData.m_created;
    bookmarkData.mountPoint = query.queryItemValue("mount_point");
    bookmarkData.locateUrl = query.queryItemValue("locate_url");
    bookmarkData.m_url = newUrl;

    m_bookmarkDataMap[newUrl.bookmarkTargetUrl()] = bookmarkData;
    m_bookmarks[newUrl.bookmarkTargetUrl()] = nullptr;

    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();
    list << QVariantMap {
        {"name", bookmarkData.m_url.bookmarkName()},
        {"url", bookmarkData.m_url.bookmarkTargetUrl()},
        {"created", bookmarkData.m_created.toString(Qt::ISODate)},
        {"lastModified", bookmarkData.m_lastModified.toString(Qt::ISODate)},
        {"mountPoint", bookmarkData.mountPoint},
        {"locateUrl", bookmarkData.locateUrl}
    };

    DFMApplication::genericSetting()->setValue("BookMark", "Items", list);
    DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::subfileCreated, bookmarkData.m_url);

    return true;
}

bool BookMarkManager::setPermissions(const QSharedPointer<DFMSetPermissionEvent> &event) const
{
    DUrl url = event->url();

    if (!url.bookmarkTargetUrl().isEmpty()) {
        return DFileService::instance()->setPermissions(event->sender(), url.bookmarkTargetUrl(), event->permissions());
    }

    return false;
}

bool BookMarkManager::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    return DFileService::instance()->deleteFiles(nullptr, {event->url()}, false);
}

BookMarkPointer BookMarkManager::findBookmark(const DUrl &url) const
{
    return m_bookmarks.value(url.bookmarkTargetUrl());
}

BookmarkData BookMarkManager::findBookmarkData(const DUrl &url) const
{
    return m_bookmarkDataMap.value(url.bookmarkTargetUrl());
}

/*!
 * \brief Update bookmark items by the given \a value
 *
 * \param value QVariant from DFMApplication::generfile:///media/wzc/asd/VM?mount_point=device:/dev/sda5&locate_url=/VMicSetting()
 *
 * Please notice that this is NOT updating bookmark list to local saved config file,
 * instead, this is updating bookmark item list FROM local saved config file.
 *
 * \sa DFMSetting::valueEdited(), DFMApplication::genericSetting()
 */
void BookMarkManager::update(const QVariant &value)
{
    const QVariantList &list = value.toList();

    DUrlList bookmarkUrlList = m_bookmarkDataMap.keys();
    for (int i = 0; i < list.count(); ++i) {
        const QVariantMap &item = list.at(i).toMap();
        const QString &name = item.value("name").toString();
        const DUrl &url = DUrl::fromUserInput(item.value("url").toString());
        const QDateTime &create_time = QDateTime::fromString(item.value("created").toString(), Qt::ISODate);
        const QDateTime &last_modified_time = QDateTime::fromString(item.value("lastModified").toString(), Qt::ISODate);
        const QString &mount_point = item.value("mountPoint").toString();
        //兼容以前未转base64版本（sp2update2之前），先判断locateUrl，保证存入bookmark中的是base64
        QByteArray ba;
        if (item.value("locateUrl").toString().startsWith("/")) {   //转base64的路径不会以'/'开头
            ba = item.value("locateUrl").toString().toLocal8Bit().toBase64();
        } else {
            ba = item.value("locateUrl").toString().toLocal8Bit();
        }
        const QString &locate_url = QString(ba);

        BookmarkData data;
        data.m_url = DUrl::fromBookMarkFile(url, name);
        data.m_created = create_time;
        data.m_lastModified = last_modified_time;
        data.mountPoint = mount_point;
        data.locateUrl = locate_url;

        if (m_bookmarkDataMap.contains(url)) {
            const BookmarkData oldData = m_bookmarkDataMap.value(url);
            m_bookmarkDataMap[url] = data;

            if (oldData.m_url.fragment() != name) {
                DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileMoved, oldData.m_url, data.m_url);

            }
        } else {
            m_bookmarkDataMap[url] = data;
            DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::subfileCreated, data.m_url);
        }

        bookmarkUrlList.removeOne(url);
    }

    for (const DUrl &url : bookmarkUrlList) {
        const BookmarkData data = m_bookmarkDataMap.value(url);
        m_bookmarks[url] = nullptr;

        DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileDeleted, data.m_url);
    }
}

void BookMarkManager::onFileEdited(const QString &group, const QString &key, const QVariant &value)
{
    if (group != "BookMark" || key != "Items")
        return;

    update(value);
}

bool BookMarkManager::onFileRenamed(const DUrl &from, const DUrl &to)
{
    //采用durl标准的转bookmarkurl接口,否则转出的URL可能isValid = false
    DUrl bookMarkFrom = DUrl::fromBookMarkFile(from, from.fileName());
    DUrl bookMarkTo = DUrl::fromBookMarkFile(to, to.fileName());

    BookmarkData data = findBookmarkData(bookMarkFrom);
    BookMarkPointer item = findBookmark(bookMarkFrom);
    if (!item || !data.m_url.isValid()) {
        return false;
    }

    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();
    for (int i = 0; i < list.count(); ++i) {
        QVariantMap map = list.at(i).toMap();

        if (map.value("name").toString() == item->getName()) {
            bookMarkFrom.setFragment(map.value("name").toString());

            QString locateUrl = to.path();
            int indexOfFirstDir = 0;
            //挂载的设备目录特殊处理
            if (locateUrl.startsWith("/media")) {
                indexOfFirstDir = locateUrl.lastIndexOf("/", locateUrl.length() - 1);
            } else {
                indexOfFirstDir = locateUrl.indexOf("/", 1);
            }
            locateUrl = locateUrl.mid(indexOfFirstDir);

            bookMarkTo.setFragment(map.value("name").toString());

            //为防止locateUrl传入QUrl被转码，locateUrl统一保存为base64
            QByteArray ba = locateUrl.toLocal8Bit().toBase64();
            map["locateUrl"] = QString(ba);
            map["url"] = bookMarkTo.path();
            list[i] = map;
            DFMApplication::genericSetting()->setValue("BookMark", "Items", list);

            BookmarkData newData;
            newData.m_url = bookMarkTo;
            newData.m_created = item->m_created;
            newData.m_lastModified = QDateTime::currentDateTime();
            newData.mountPoint = map.value("mountPoint").toString();
            newData.locateUrl = QString(ba);

            BookMarkPointer new_item(new BookMark(bookMarkTo));
            new_item->m_created = item->m_created;
            new_item->m_lastModified = QDateTime::currentDateTime();
            new_item->mountPoint = map.value("mountPoint").toString();
            new_item->locateUrl = QString(ba);

            m_bookmarkDataMap.remove(bookMarkFrom.bookmarkTargetUrl());
            m_bookmarkDataMap[bookMarkTo.bookmarkTargetUrl()] = newData;
            m_bookmarks.remove(bookMarkFrom.bookmarkTargetUrl());
            m_bookmarks[bookMarkTo.bookmarkTargetUrl()] = new_item;

            DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileMoved, bookMarkFrom, bookMarkTo);
            return true;
        }
    }

    return false;
}

void BookMarkManager::refreshBookmark()
{
    update(DFMApplication::genericSetting()->value("BookMark", "Items"));
}

const DUrlList BookMarkManager::getBookmarkUrls()
{
    DUrlList list;
    for (const BookmarkData &data : m_bookmarkDataMap) {
        list.append(data.m_url);
    }

    return list;
}

const QList<DAbstractFileInfoPointer> BookMarkManager::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    ///这个函数目前不会被调用了
    Q_UNUSED(event);
    QList<DAbstractFileInfoPointer> infolist;

    for (BookMarkPointer bk : m_bookmarks) {
        infolist.append(DAbstractFileInfoPointer(bk));
    }

    return infolist;
}

const DAbstractFileInfoPointer BookMarkManager::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    if (event->fileUrl() == DUrl(BOOKMARK_ROOT)) {
        return DAbstractFileInfoPointer(new BookMark(DUrl(BOOKMARK_ROOT)));
    }

    DUrl bookmarkUrl = event->url().bookmarkTargetUrl();
    if (!m_bookmarkDataMap.contains(bookmarkUrl)) {
        DUrl targetUrl = event->url().bookmarkTargetUrl();
        if (targetUrl.scheme().isEmpty()) {
            targetUrl.setScheme(FILE_SCHEME);
        }
        return DFileService::instance()->createFileInfo(event->sender(), targetUrl);
    }

    BookMarkPointer bp = findBookmark(event->url());
    if (!bp) {
        BookmarkData data = m_bookmarkDataMap[bookmarkUrl];
        BookMarkPointer item(new BookMark(event->url()));
        item->m_created = data.m_created;
        item->m_lastModified = data.m_lastModified;
        item->mountPoint = data.mountPoint;
        item->locateUrl = data.locateUrl;
        m_bookmarks[item->sourceUrl()] = item;
        return item;
    }

    return bp;
}

DAbstractFileWatcher *BookMarkManager::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    if (event->url() != DUrl(BOOKMARK_ROOT)) {
        return nullptr;
    }

    return new BookMarkFileWatcher(event->url());
}
