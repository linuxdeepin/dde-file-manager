/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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

#include "recentmanager.h"
#include "recentfileinfo.h"
#include "recentiterateworker.h"
#include "recentfilewatcher.h"
#include "events/recenteventcaller.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "dfm-base/base/schemefactory.h"

#include <QFile>
#include <QMenu>

DPRECENT_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

static constexpr char kEmptyRecentFile[] =
        R"|(<?xml version="1.0" encoding="UTF-8"?>
        <xbel version="1.0"
        xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks"
        xmlns:mime="http://www.freedesktop.org/standards/shared-mime-info"
        >
        </xbel>)|";

RecentManager *RecentManager::instance()
{
    static RecentManager instance;
    return &instance;
}

QUrl RecentManager::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath("/");
    return url;
}

void RecentManager::clearRecent()
{

    QFile f(xbelPath());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(kEmptyRecentFile);
        f.close();
    } else {
        qWarning() << "open recent xbel file failed!!!";
    }
}

void RecentManager::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    QMenu *menu = new QMenu;
    menu->addAction(QObject::tr("Open in new window"), [url]() {
        RecentEventCaller::sendOpenWindow(url);
    });

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        RecentEventCaller::sendOpenTab(windowId, url);
    });

    auto &ctx = dpfInstance.serviceContext();
    auto workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());
    if (!workspaceService) {
        qCritical() << "Failed, recentManager contenxtMenuHandle \"WorkspaceService\" is empty";
        abort();
    }

    newTabAct->setDisabled(!workspaceService->tabAddable(windowId));

    menu->addSeparator();
    menu->addAction(QObject::tr("Clear recent history"), [url]() {
        RecentManager::clearRecent();
    });
    menu->exec(globalPos);
    delete menu;
}

bool RecentManager::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    Q_UNUSED(error)

    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(SchemeTypes::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendOpenFiles(windowId, redirectedFileUrls);
    return true;
}

bool RecentManager::writeToClipBoardHandle(const quint64 windowId, const ClipBoard::ClipboardAction action, const QList<QUrl> urls)
{
    QList<QUrl> redirectedFileUrls;
    for (QUrl url : urls) {
        url.setScheme(SchemeTypes::kFile);
        redirectedFileUrls << url;
    }
    RecentEventCaller::sendWriteToClipboard(windowId, action, redirectedFileUrls);
    return true;
}

QMap<QUrl, AbstractFileInfoPointer> RecentManager::getRecentNodes() const
{
    return recentNodes;
}

bool RecentManager::removeRecentFile(const QUrl &url)
{
    if (recentNodes.contains(url)) {
        recentNodes.remove(url);
        return true;
    }
    return false;
}

RecentManager::RecentManager(QObject *parent)
    : QObject(parent)
{
    init();
}

RecentManager::~RecentManager()
{
    if (watcher)
        watcher->stopWatcher();
    workerThread.quit();
    workerThread.wait();
}

void RecentManager::init()
{
    RecentIterateWorker *worker = new RecentIterateWorker;
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &RecentManager::asyncHandleFileChanged, worker, &RecentIterateWorker::doWork);

    connect(worker, &RecentIterateWorker::updateRecentFileInfo, this,
            &RecentManager::onUpdateRecentFileInfo, Qt::BlockingQueuedConnection);
    connect(worker, &RecentIterateWorker::deleteExistRecentUrls, this,
            &RecentManager::onDeleteExistRecentUrls, Qt::BlockingQueuedConnection);

    workerThread.start();

    emit asyncHandleFileChanged();
    updateRecentTimer.setSingleShot(true);
    updateRecentTimer.setInterval(300);
    updateRecentTimer.moveToThread(qApp->thread());

    connect(&updateRecentTimer, &QTimer::timeout, this, &RecentManager::asyncHandleFileChanged);

    watcher = WacherFactory::create<AbstractFileWatcher>(QUrl::fromLocalFile(RecentManager::xbelPath()));
    connect(watcher.data(), &AbstractFileWatcher::subfileCreated, this, &RecentManager::updateRecent);
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentManager::updateRecent);
    watcher->startWatcher();
}

void RecentManager::updateRecent()
{
    updateRecentTimer.start();
}

void RecentManager::onUpdateRecentFileInfo(const QUrl &url, qint64 readTime)
{
    if (!recentNodes.contains(url)) {
        recentNodes[url] = InfoFactory::create<AbstractFileInfo>(url);
        QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentManager::rootUrl());
        if (watcher) {
            emit watcher->subfileCreated(url);
        }
    }

    //ToDo(yanghao):update read time
    Q_UNUSED(readTime)
}

void RecentManager::onDeleteExistRecentUrls(QList<QUrl> &urls)
{
    for (const auto &url : urls) {
        if (removeRecentFile(url)) {
            QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentManager::rootUrl());
            if (watcher) {
                emit watcher->fileDeleted(url);
            }
        }
    }
}
