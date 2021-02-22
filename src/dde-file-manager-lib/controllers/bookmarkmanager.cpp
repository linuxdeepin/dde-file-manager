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
    update(DFMApplication::genericSetting()->value("BookMark", "Items"));

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
    BookMarkPointer p = m_bookmarks.value(url.bookmarkTargetUrl());
    return p;
}

bool BookMarkManager::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    DUrl from = event->fromUrl();
    DUrl new_from = from.bookmarkTargetUrl();
    DUrl to = event->toUrl();

    BookMarkPointer item = findBookmark(event->fromUrl());

    if (!item) {
        return false;
    }

    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();

    for (int i = 0; i < list.count(); ++i) {
        QVariantMap map = list.at(i).toMap();

        if (map.value("name").toString() == item->getName()) {
            map["name"] = event->toUrl().bookmarkName();
            list[i] = map;

            DFMApplication::genericSetting()->setValue("BookMark", "Items", list);
            BookMark *new_item = new BookMark(event->toUrl());
            QUrlQuery query(event->toUrl());

            new_item->m_created = item->m_created;
            new_item->m_lastModified = QDateTime::currentDateTime();
            new_item->mountPoint = item->getMountPoint();//query.queryItemValue("mount_point");
            new_item->locateUrl = map.value("locateUrl").toString();//query.queryItemValue("locate_url");

            m_bookmarks[event->toUrl().bookmarkTargetUrl()] = new_item;
            break;
        }
    }

    DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileMoved, event->fromUrl(), event->toUrl());

    return true;
}

bool BookMarkManager::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();

    for (const DUrl url : event->urlList()) {
        const BookMarkPointer &info = m_bookmarks.take(url.bookmarkTargetUrl());

        if (!info)
            continue;

        for (int i = 0; i < list.count(); ++i) {
            const QVariantMap &map = list.at(i).toMap();

            if (map.value("name").toString() == info->getName()) {
                list.removeAt(i);
                break;
            }
        }

        DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileDeleted, info->fileUrl());
    }

    DFMApplication::genericSetting()->setValue("BookMark", "Items", list);

    return true;
}

bool BookMarkManager::touch(const QSharedPointer<DFMTouchFileEvent> &event) const
{
    //与其他书签储存逻辑保持一致，去掉url中的Query字符串
    DUrl newUrl = event->url();
    newUrl.setQuery("");
    BookMarkPointer item(new BookMark(newUrl));
    QUrlQuery query(event->url());

    item->m_created = QDateTime::currentDateTime();
    item->m_lastModified = item->m_created;
    item->mountPoint = query.queryItemValue("mount_point");
    item->locateUrl = query.queryItemValue("locate_url");
    m_bookmarks[item->sourceUrl()] = item;


    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();

    list << QVariantMap {
        {"name", item->getName()},
        {"url", item->sourceUrl()},
        {"created", item->m_created.toString(Qt::ISODate)},
        {"lastModified", item->m_lastModified.toString(Qt::ISODate)},
        {"mountPoint", item->mountPoint},
        {"locateUrl", item->locateUrl}
    };

    DFMApplication::genericSetting()->setValue("BookMark", "Items", list);
    DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::subfileCreated, item->fileUrl());

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

    DUrlList bookmarkUrlList = m_bookmarks.keys();

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
        }
        else {
            ba = item.value("locateUrl").toString().toLocal8Bit();
        }
        const QString &locate_url = QString(ba);
        if (DFileService::instance()->checkGvfsMountfileBusy(url,false))
            continue;
        BookMark *bm_info = new BookMark(name, url);

        bm_info->m_created = create_time;
        bm_info->m_lastModified = last_modified_time;
        bm_info->mountPoint = mount_point;
        bm_info->locateUrl = locate_url;

        if (m_bookmarks.contains(url)) {
            const BookMarkPointer old_info = m_bookmarks.value(url);

            m_bookmarks[url] = BookMarkPointer(bm_info);

            if (old_info->getName() != name) {
                DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileMoved, old_info->fileUrl(), bm_info->fileUrl());
            }
        } else {
            m_bookmarks[url] = BookMarkPointer(bm_info);

            DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::subfileCreated, bm_info->fileUrl());
        }

        bookmarkUrlList.removeOne(url);
    }

    for (const DUrl &url : bookmarkUrlList) {
        const BookMarkPointer &info = m_bookmarks.take(url);

        DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileDeleted, info->fileUrl());
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
    //make bookMarkDUrl
//    QString fromPath = from.scheme() + "://" + from.path();
//    DUrl bookMarkFrom(from);
//    bookMarkFrom.setScheme(BOOKMARK_SCHEME);
//    bookMarkFrom.setPath(fromPath);
    //采用durl标准的转bookmarkurl接口,否则转出的URL可能isValid = false
    DUrl bookMarkFrom = DUrl::fromBookMarkFile(from, from.fileName());

//    QString toPath = to.scheme() + "://" + to.path();
//    DUrl bookMarkTo(to);
//    bookMarkTo.setScheme(BOOKMARK_SCHEME);
//    bookMarkTo.setPath(toPath);
    //采用durl标准的转bookmarkurl接口,否则转出的URL可能isValid = false
    DUrl bookMarkTo = DUrl::fromBookMarkFile(to, to.fileName());

    BookMarkPointer item = findBookmark(bookMarkFrom);
    if (!item) {
        return false;
    }

    QVariantList list = DFMApplication::genericSetting()->value("BookMark", "Items").toList();
    for (int i = 0; i < list.count(); ++i) {
        QVariantMap map = list.at(i).toMap();

        if (map.value("name").toString() == item->getName()) {
//            QString fromQueryStr = "mount_point=" + map.value("mountPoint").toString() + "&locate_url=" + map.value("locateUrl").toString();
//            bookMarkFrom.setQuery(fromQueryStr);
            bookMarkFrom.setFragment(map.value("name").toString());

//            QString locateUrl = bookMarkTo.bookmarkTargetUrl().toLocalFile();
            QString locateUrl = to.path();
            int indexOfFirstDir = 0;
            //挂载的设备目录特殊处理
            if (locateUrl.startsWith("/media")) {
                indexOfFirstDir = locateUrl.lastIndexOf("/", locateUrl.length() - 1);
            } else {
                indexOfFirstDir = locateUrl.indexOf("/", 1);
            }
            locateUrl = locateUrl.mid(indexOfFirstDir);

//            QString toQueryStr = "mount_point=" + map.value("mountPoint").toString() + "&locate_url=" + locateUrl;
//            bookMarkTo.setQuery(toQueryStr);
            bookMarkTo.setFragment(map.value("name").toString());

            //为防止locateUrl传入QUrl被转码，locateUrl统一保存为base64
            QByteArray ba = locateUrl.toLocal8Bit().toBase64();
            map["locateUrl"] = QString(ba);
            map["url"] = bookMarkTo.path();
            list[i] = map;
            DFMApplication::genericSetting()->setValue("BookMark", "Items", list);

            BookMarkPointer new_item(new BookMark(bookMarkTo));
            new_item->m_created = item->m_created;
            new_item->m_lastModified = QDateTime::currentDateTime();
            new_item->mountPoint = map.value("mountPoint").toString();
            new_item->locateUrl = QString(ba);

            m_bookmarks.remove(bookMarkFrom.bookmarkTargetUrl());
            m_bookmarks[bookMarkTo.bookmarkTargetUrl()] = new_item;

            DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileMoved, bookMarkFrom, bookMarkTo);
            return true;
        }
    }

    return false;
}

const QList<DAbstractFileInfoPointer> BookMarkManager::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
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

    BookMarkPointer bp = findBookmark(event->url());
    if (!bp) {
        DUrl targetUrl = event->url().bookmarkTargetUrl();
        if (targetUrl.scheme().isEmpty()) {
            targetUrl.setScheme(FILE_SCHEME);
        }
        return DFileService::instance()->createFileInfo(event->sender(), targetUrl);
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
