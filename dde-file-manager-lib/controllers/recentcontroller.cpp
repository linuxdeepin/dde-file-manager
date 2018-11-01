/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#include "recentcontroller.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dfmevent.h"
#include "private/dabstractfilewatcher_p.h"

#include <QFileSystemWatcher>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QDebug>

class RecentFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    RecentFileWatcherPrivate(DAbstractFileWatcher *qq)
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

class RecentFileWatcher : public DAbstractFileWatcher
{
public:
    explicit RecentFileWatcher(const DUrl &url, QObject *parent = nullptr)
        : DAbstractFileWatcher(*new RecentFileWatcherPrivate(this), url, parent)
    {
    }
};

RecentController::RecentController(QObject *parent)
    : DAbstractFileController(parent)
{
    QFileSystemWatcher *watcher = new QFileSystemWatcher;
    watcher->addPath(QDir::homePath() + "/.local/share/recently-used.xbel");

    auto handleFileChanged = [=] {
        QFile file(QDir::homePath() + "/.local/share/recently-used.xbel");

        if (file.open(QIODevice::ReadOnly)) {
            QXmlStreamReader reader(&file);

            while (!reader.atEnd()) {
                if (!reader.readNextStartElement() ||
                     reader.name() != "bookmark") {
                    continue;
                }

                const QStringRef &location = reader.attributes().value("href");
                const QStringRef &added = reader.attributes().value("added");

                if (!location.isEmpty()) {
                    DUrl url = DUrl(location.toString());
                    QFileInfo info(url.toLocalFile());
                    DUrl recentUrl = url;
                    recentUrl.setScheme(RECENT_SCHEME);

                    if (info.exists() && info.isFile()) {
                        if (!m_recentNodes.contains(recentUrl)) {
                            RecentFileInfo *fileInfo = new RecentFileInfo(recentUrl);
                            m_recentNodes[recentUrl] = fileInfo;

                            DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT),
                                                              &DAbstractFileWatcher::subfileCreated,
                                                              recentUrl);
                        }
                    }
                }
            }
        }

        watcher->addPath(QDir::homePath() + "/.local/share/recently-used.xbel");
    };

    handleFileChanged();
    connect(watcher, &QFileSystemWatcher::fileChanged, this, handleFileChanged, Qt::QueuedConnection);
}

bool RecentController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    return DFileService::instance()->openFileLocation(event->sender(), DUrl::fromLocalFile(event->url().path()));
}

bool RecentController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), DUrl::fromLocalFile(event->url().path()));
}

bool RecentController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    return DFileService::instance()->openFileByApp(event->sender(), event->appName(), DUrl::fromLocalFile(event->url().path()));
}

bool RecentController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    DUrlList list;
    for (const DUrl &url : event->urlList()) {
        list << DUrl::fromLocalFile(url.path());
    }

    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(), list);
}

bool RecentController::compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const
{
    DUrlList list;
    for (const DUrl &url : event->urlList()) {
        list << DUrl::fromLocalFile(url.path());
    }

    return DFileService::instance()->compressFiles(event->sender(), list);
}

bool RecentController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    return DFileService::instance()->createSymlink(event->sender(), DUrl::fromLocalFile(event->fileUrl().path()), event->toUrl());
}

bool RecentController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList list;
    for (const DUrl &url : event->urlList()) {
        list << DUrl::fromLocalFile(url.path());
    }

    QFile file(QDir::homePath() + "/.local/share/recently-used.xbel");
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    auto node_list = root.elementsByTagName("bookmark");

    for (int i = 0; i < node_list.count(); ) {
        const DUrl fileUrl = DUrl(node_list.at(i).toElement().attribute("href"));

        if (list.contains(fileUrl)) {
            root.removeChild(node_list.at(i));

            DUrl recentUrl(fileUrl);
            recentUrl.setScheme(RECENT_SCHEME);
            m_recentNodes.remove(recentUrl);

            DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT),
                                              &DAbstractFileWatcher::fileDeleted,
                                              recentUrl);
        } else {
            ++i;
        }
    }

    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QTextStream out(&file);
    out << doc.toString();

    return true;
}

bool RecentController::setFileTags(const QSharedPointer<DFMSetFileTagsEvent> &event) const
{
    if (!event->url().isValid()) {
        return false;
    }

    QList<QString> tags = event->tags();
    return DFileService::instance()->setFileTags(this, DUrl::fromLocalFile(event->fileUrl().path()), tags);
}

bool RecentController::removeTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event) const
{
    if (!event->url().isValid()) {
        return false;
    }

    return DFileService::instance()->removeTagsOfFile(this, DUrl::fromLocalFile(event->fileUrl().path()), event->tags());
}

QList<QString> RecentController::getTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event) const
{
    QList<DUrl> list = event->urlList();

    for (DUrl &item : list) {
        item = DUrl::fromLocalFile(item.path());
    }

    return DFileService::instance()->getTagsThroughFiles(this, list);
}

const QList<DAbstractFileInfoPointer> RecentController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    Q_UNUSED(event)

    QList<DAbstractFileInfoPointer> list;

    for (RecentPointer rp : m_recentNodes) {
        list.append(DAbstractFileInfoPointer(rp));
    }

    return list;
}

const DAbstractFileInfoPointer RecentController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    if (event->url() == DUrl(RECENT_ROOT)) {
        return DAbstractFileInfoPointer(new RecentFileInfo(event->url()));
    }

    return DAbstractFileInfoPointer(m_recentNodes.value(event->url()));
}

DAbstractFileWatcher *RecentController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new RecentFileWatcher(event->url());
}
