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
#include <QQueue>
#include <QTimer>
#include <QDebug>

#include <DRecentManager>

DCORE_USE_NAMESPACE

class RecentFileWatcherPrivate;
class RecentFileWatcher : public DAbstractFileWatcher
{
public:
    explicit RecentFileWatcher(const DUrl &url, QObject *parent = nullptr);

    void setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled = true) override;

private:
    void addWatcher(const DUrl &url);
    void removeWatcher(const DUrl &url);

    void onFileDeleted(const DUrl &url);
    void onFileAttributeChanged(const DUrl &url);
    void onFileModified(const DUrl &url);

    Q_DECLARE_PRIVATE(RecentFileWatcher)
};

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

    QMap<DUrl, DAbstractFileWatcher *> urlToWatcherMap;

    Q_DECLARE_PUBLIC(RecentFileWatcher)
};

RecentFileWatcher::RecentFileWatcher(const DUrl &url, QObject *parent)
    : DAbstractFileWatcher(*new RecentFileWatcherPrivate(this), url, parent)
{

}

void RecentFileWatcher::setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled)
{
    if (!subfileUrl.isRecentFile())
        return;

    if (enabled) {
        addWatcher(subfileUrl);
    } else {
        removeWatcher(subfileUrl);
    }
}

void RecentFileWatcher::addWatcher(const DUrl &url)
{
    Q_D(RecentFileWatcher);

    if (!url.isValid() || d->urlToWatcherMap.contains(url)) {
        return;
    }

    DUrl real_url = url;
    real_url.setScheme(FILE_SCHEME);

    DAbstractFileWatcher *watcher = DFileService::instance()->createFileWatcher(this, real_url);

    if (!watcher)
        return;

    watcher->moveToThread(this->thread());
    watcher->setParent(this);

    connect(watcher, &DAbstractFileWatcher::fileAttributeChanged, this, &RecentFileWatcher::onFileAttributeChanged);
    connect(watcher, &DAbstractFileWatcher::fileDeleted, this, &RecentFileWatcher::onFileDeleted);
    connect(watcher, &DAbstractFileWatcher::fileModified, this, &RecentFileWatcher::onFileModified);

    d->urlToWatcherMap[url] = watcher;

    if (d->started) {
        watcher->startWatcher();
    }
}

void RecentFileWatcher::removeWatcher(const DUrl &url)
{
    Q_D(RecentFileWatcher);

    DAbstractFileWatcher *watcher = d->urlToWatcherMap.take(url);

    if (!watcher) {
        return;
    }

    watcher->deleteLater();
}

void RecentFileWatcher::onFileDeleted(const DUrl &url)
{
    removeWatcher(url);

    DUrl newUrl = url;
    newUrl.setScheme(RECENT_SCHEME);

    emit fileDeleted(newUrl);
}

void RecentFileWatcher::onFileAttributeChanged(const DUrl &url)
{
    DUrl newUrl = url;
    newUrl.setScheme(RECENT_SCHEME);

    emit fileAttributeChanged(newUrl);
}

void RecentFileWatcher::onFileModified(const DUrl &url)
{
    DUrl newUrl = url;
    newUrl.setScheme(RECENT_SCHEME);

    emit fileModified(newUrl);
}

class RecentDirIterator : public DDirIterator
{
public:
    RecentDirIterator(const DUrl &url, const QStringList &nameFilters, QDir::Filters filter,
                      QDirIterator::IteratorFlags flags, RecentController *parent);

    DUrl next() override;
    bool hasNext() const override;

    QString fileName() const override;
    DUrl fileUrl() const override;
    const DAbstractFileInfoPointer fileInfo() const override;
    DUrl url() const override;

private:
    DAbstractFileInfoPointer m_currentFileInfo;
    RecentController *parent;

    mutable QQueue<DUrl> urlList;
    DUrl m_url;
};

RecentDirIterator::RecentDirIterator(const DUrl &url, const QStringList &nameFilters,
                                     QDir::Filters filter, QDirIterator::IteratorFlags flags,
                                     RecentController *parent)
    : DDirIterator(),
      parent(parent)
{
    Q_UNUSED(url)
    Q_UNUSED(nameFilters)
    Q_UNUSED(filter)
    Q_UNUSED(flags)

    for (DUrl url : parent->recentNodes.keys()) {
        urlList << url;
    }
}

DUrl RecentDirIterator::next()
{
    if (!urlList.isEmpty()) {
        m_url = urlList.dequeue();
        return m_url;
    }

    return DUrl();
}

bool RecentDirIterator::hasNext() const
{
    if (!urlList.isEmpty()) {
        return true;
    }

    return false;
}

QString RecentDirIterator::fileName() const
{
    DAbstractFileInfoPointer currentInfo = parent->recentNodes.value(m_url);

    return currentInfo ? currentInfo->fileName() : QString();
}

DUrl RecentDirIterator::fileUrl() const
{
    DAbstractFileInfoPointer currentInfo = parent->recentNodes.value(m_url);

    return currentInfo ? currentInfo->fileUrl() : DUrl();
}

const DAbstractFileInfoPointer RecentDirIterator::fileInfo() const
{
    return parent->recentNodes.value(m_url);
}

DUrl RecentDirIterator::url() const
{
    return DUrl(RECENT_ROOT);
}

RecentController::RecentController(QObject *parent)
    : DAbstractFileController(parent),
      m_watcher(new QFileSystemWatcher),
      m_refreshTimer(new QTimer),
      m_xbelPath(QDir::homePath() + "/.local/share/recently-used.xbel")
{
    // add directory.
    m_watcher->addPath(QDir::homePath() + "/.local/share");

    m_refreshTimer->setInterval(100);
    m_refreshTimer->setSingleShot(true);

    handleFileChanged();

    connect(m_refreshTimer, &QTimer::timeout, this, &RecentController::handleFileChanged);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, m_refreshTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &RecentController::handleDirectoryChanged);
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
    return DFileService::instance()->writeFilesToClipboard(event->sender(), event->action(),
                                                           realUrlList(event->urlList()));
}

bool RecentController::compressFiles(const QSharedPointer<DFMCompressEvnet> &event) const
{
    return DFileService::instance()->compressFiles(event->sender(), realUrlList(event->urlList()));
}

bool RecentController::decompressFile(const QSharedPointer<DFMDecompressEvnet> &event) const
{
    return DFileService::instance()->decompressFile(event->sender(), realUrlList(event->urlList()));
}

bool RecentController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    return DFileService::instance()->createSymlink(event->sender(), DUrl::fromLocalFile(event->fileUrl().path()), event->toUrl());
}

bool RecentController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    QStringList list;
    for (const DUrl &url : event->urlList()) {
        list << DUrl::fromLocalFile(url.path()).toString();
    }

    DRecentManager::removeItems(list);

    return true;
}

DUrlList RecentController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    DFileService::instance()->deleteFiles(event->sender(), event->urlList(), false, false, true);

    return DUrlList();
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

const DDirIteratorPointer RecentController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    RecentDirIterator *iterator = new RecentDirIterator(event->url(), event->nameFilters(),
                                                        event->filters(), event->flags(),
                                                        const_cast<RecentController *>(this));
    return DDirIteratorPointer(iterator);
}

const DAbstractFileInfoPointer RecentController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    if (event->url() == DUrl(RECENT_ROOT)) {
        return DAbstractFileInfoPointer(new RecentFileInfo(event->url()));
    }

    return DAbstractFileInfoPointer(recentNodes.value(event->url()));
}

DAbstractFileWatcher *RecentController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new RecentFileWatcher(event->url());
}

DUrlList RecentController::realUrlList(const DUrlList &recentUrls)
{
    DUrlList list;

    for (const DUrl &url : recentUrls) {
        list << DUrl::fromLocalFile(url.path());
    }

    return list;
}

void RecentController::handleFileChanged()
{
    // read xbel file.
    QFile file(m_xbelPath);
    QList<DUrl> urlList;

    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);

        while (!reader.atEnd()) {
            if (!reader.readNextStartElement() ||
                 reader.name() != "bookmark") {
                continue;
            }

            const QStringRef &location = reader.attributes().value("href");

            if (!location.isEmpty()) {
                DUrl url = DUrl(location.toString());
                QFileInfo info(url.toLocalFile());
                DUrl recentUrl = url;
                recentUrl.setScheme(RECENT_SCHEME);

                if (info.exists() && info.isFile()) {
                    urlList << recentUrl;

                    if (!recentNodes.contains(recentUrl)) {
                        RecentFileInfo *fileInfo = new RecentFileInfo(recentUrl);
                        recentNodes[recentUrl] = fileInfo;

                        DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT),
                                                          &DAbstractFileWatcher::subfileCreated,
                                                          recentUrl);
                    }
                }
            }
        }
    }

    // delete does not exist url.
    for (auto iter = recentNodes.begin(); iter != recentNodes.end(); ) {
        DUrl url = iter.key();

        if (!urlList.contains(url)) {
            iter = recentNodes.erase(iter);

            DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT),
                                              &DAbstractFileWatcher::fileDeleted,
                                              url);
        } else {
            iter.value()->updateInfo();

            ++iter;
        }
    }

    m_watcher->addPath(m_xbelPath);
}

void RecentController::handleDirectoryChanged()
{
    m_refreshTimer->start();

    if (QFileInfo(m_xbelPath).exists()) {
        m_watcher->addPath(m_xbelPath);
    } else {
        m_watcher->removePath(m_xbelPath);
    }
}
