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

#include "app/define.h"

#include <QJsonObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QDateTime>
#include <QUrl>

#include <stdlib.h>

#include "shutil/fileutils.h"
#include "views/dbookmarkitem.h"
#include "deviceinfo/udisklistener.h"

class BookMarkFileWatcher;
class BookMarkFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    BookMarkFileWatcherPrivate(DAbstractFileWatcher *qq)
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
    , BaseManager()
{
    load();
    fileService->setFileUrlHandler(BOOKMARK_SCHEME, "", this);
}

BookMarkManager::~BookMarkManager()
{

}

void BookMarkManager::load()
{
    //Migration for old config files, and rmove that codes for further
    FileUtils::migrateConfigFileFromCache("bookmark");

    //TODO: check permission and existence of the path
    QString configPath = cachePath();
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't open bookmark file!";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    loadJson(jsonDoc.object());
    file.close();
}

void BookMarkManager::save() const
{
    //TODO: check permission and existence of the path
    QString configPath = cachePath();
    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't write bookmark file!";
        return;
    }
    QJsonObject object;
    writeJson(object);
    QJsonDocument jsonDoc(object);
    file.write(jsonDoc.toJson());
    file.close();
}

int BookMarkManager::getBookmarkIndex(const DUrl &url)
{
    for (int i = 0; i <= m_bookmarks.count(); i++) {
        if (m_bookmarks[i]->fileUrl() == url) {
            return i;
        }
    }

    return -1;
}

QList<BookMarkPointer> BookMarkManager::getBookmarks()
{
    return m_bookmarks;
}

bool BookMarkManager::renameFile(const QSharedPointer<DFMRenameEvent> &event) const
{
    BookMarkPointer item = findBookmark(event->fromUrl());
    if (!item) {
        return false;
    }
    item->setName(event->toUrl().bookmarkName());
    DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileMoved, event->fromUrl(), event->toUrl());
    save();

    return true;
}

bool BookMarkManager::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    for (const DUrl url : event->urlList()) {
        BookMarkPointer item = findBookmark(url);

        if (!item) {
            continue;
        }

        const_cast<BookMarkManager *>(this)->m_bookmarks.removeOne(item);
        item.reset();

        DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::fileDeleted, url);
    }

    save();

    return true;
}

bool BookMarkManager::touch(const QSharedPointer<DFMTouchFileEvent> &event) const
{
    BookMarkPointer item(new BookMark(event->url()));
    const_cast<BookMarkManager *>(this)->m_bookmarks.append(item);
    save();

    DAbstractFileWatcher::ghostSignal(DUrl(BOOKMARK_ROOT), &DAbstractFileWatcher::subfileCreated, item->fileUrl());

    return true;
}

//void BookMarkManager::appendBookmark(QExplicitlySharedDataPointer<BookMark> bookmarkPointer)noexcept
//{
//    m_bookmarks.append(bookmarkPointer);
//}

//void BookMarkManager::appendTagBookmark(QExplicitlySharedDataPointer<BookMark> bookmark) noexcept
//{
//    m_tagBookmarks.push_back(bookmark);
//}

//void BookMarkManager::clearTagBookmark()
//{
//    if(!m_tagBookmarks.empty()){
//        m_tagBookmarks.clear();
//    }
//}

QString BookMarkManager::cachePath()
{
    return getConfigPath("bookmark");
}

void BookMarkManager::loadJson(const QJsonObject &json)
{
    QJsonArray jsonArray = json["Bookmark"].toArray();
    for (int i = 0; i < jsonArray.size(); i++) {
        QJsonObject object = jsonArray[i].toObject();
        QString time = object["t"].toString();
        QString name = object["n"].toString();
        QString url = object["u"].toString();

        QString deviceID = object["deviceID"].toString();
        if (deviceID.isEmpty()) {
            DFileInfo info(url);
            deviceID = QDiskInfo::getDiskInfo(info).id();
        }
        QString uuid = object["uuid"].toString();
        BookMarkPointer bm(new BookMark(QDateTime::fromString(time), name, DUrl(url)));
        bm->setDevcieId(deviceID);
        bm->setUuid(uuid);
        m_bookmarks.append(bm);
    }
}

void BookMarkManager::writeJson(QJsonObject &json) const
{
    QJsonArray localArray;
    for (int i = 0; i < m_bookmarks.size(); i++) {
        QJsonObject object;
        object["t"] = m_bookmarks.at(i)->getDateTime().toString();
        object["n"] = m_bookmarks.at(i)->getName();
        object["u"] = m_bookmarks.at(i)->sourceUrl().toString();

        QString deviceID = QDiskInfo::getDiskInfo(*m_bookmarks.at(i).data()).id();
        if (deviceID.isEmpty()) {
            DFileInfo info(m_bookmarks.at(i)->sourceUrl());
            deviceID = QDiskInfo::getDiskInfo(info).id();
        }
        if (deviceID.isEmpty()) {
            deviceID = m_bookmarks.at(i)->getDevcieId();
        }

        m_bookmarks.at(i)->setDevcieId(deviceID);
        object["deviceID"] = deviceID;

        QString uuid = QDiskInfo::getDiskInfo(*m_bookmarks.at(i).data()).uuid();
        object["uuid"] = uuid;

        localArray.append(object);
    }
    json["Bookmark"] = localArray;
}

BookMarkPointer BookMarkManager::findBookmark(const DUrl &url) const
{
    for (const BookMarkPointer &item : m_bookmarks) {
        if (item->fileUrl() == url) {
            return item;
        }
    }

    return BookMarkPointer();
}

void BookMarkManager::removeBookmark(BookMarkPointer bookmark)
{
    foreach (BookMarkPointer p, m_bookmarks) {
        if (p->getDateTime() == bookmark->getDateTime() && p->getName() == bookmark->getName()) {
            m_bookmarks.removeOne(p);
            break;
        }
    }
    save();
}

void BookMarkManager::renameBookmark(BookMarkPointer bookmark, const QString &newname)
{
    bookmark->setName(newname);
    save();
}

void BookMarkManager::moveBookmark(int from, int to)
{
    if (from == to) {
        return;
    }

    m_bookmarks.move(from, to);
    save();
}

void BookMarkManager::reLoad()
{
    m_bookmarks.clear();
    load();
}

const QList<DAbstractFileInfoPointer> BookMarkManager::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    Q_UNUSED(event);
    QList<DAbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_bookmarks.size(); i++) {
        infolist.append(DAbstractFileInfoPointer(m_bookmarks.at(i)));
    }

    return infolist;
}

const DAbstractFileInfoPointer BookMarkManager::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new BookMark(event->url()));
}

DAbstractFileWatcher *BookMarkManager::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    if (event->url() != DUrl(BOOKMARK_ROOT)) {
        return nullptr;
    }

    return new BookMarkFileWatcher(event->url());
}
