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
#include "files/recentfileinfo.h"
#include "files/recentiterateworker.h"
#include "files/recentfilewatcher.h"
#include "events/recenteventcaller.h"

#include "services/filemanager/workspace/workspaceservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/base/device/deviceproxymanager.h"

#include <QFile>
#include <QMenu>
#include <QCoreApplication>

DPRECENT_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPF_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

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

RecentManager::ExpandFieldMap RecentManager::propetyExtensionFunc(const QUrl &url)
{
    using BasicExpandType = DSC_NAMESPACE::CPY_NAMESPACE::BasicExpandType;
    using BasicExpand = DSC_NAMESPACE::CPY_NAMESPACE::BasicExpand;
    using BasicFieldExpandEnum = DSC_NAMESPACE::CPY_NAMESPACE::BasicFieldExpandEnum;

    BasicExpand expand;
    const auto &info = InfoFactory::create<AbstractFileInfo>(url);
    const QString &sourcePath = info->redirectedFileUrl().toLocalFile();
    expand.insert(BasicFieldExpandEnum::kFileModifiedTime, qMakePair(QObject::tr("Source path"), sourcePath));

    ExpandFieldMap map;
    map[BasicExpandType::kFieldInsert] = expand;

    return map;
}

QUrl RecentManager::urlTransform(const QUrl &url)
{
    QUrl out { url };
    out.setScheme(Global::kFile);
    return out;
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

bool RecentManager::customColumnRole(const QUrl &rootUrl, QList<ItemRoles> *roleList)
{
    if (rootUrl.scheme() == scheme()) {
        roleList->append(kItemNameRole);
        roleList->append(kItemFilePathRole);
        roleList->append(kItemFileLastReadRole);
        roleList->append(kItemFileSizeRole);
        roleList->append(kItemFileMimeTypeRole);

        return true;
    }

    return false;
}

bool RecentManager::customRoleDisplayName(const QUrl &url, const ItemRoles role, QString *displayName)
{
    if (url.scheme() != scheme())
        return false;

    if (role == kItemFilePathRole) {
        displayName->append(tr("Path"));
        return true;
    }

    if (role == kItemFileLastReadRole) {
        displayName->append(tr("Last access"));
        return true;
    }

    return false;
}

bool RecentManager::customRoleData(const QUrl &rootUrl, const QUrl &url, const ItemRoles role, QVariant *data)
{
    Q_UNUSED(rootUrl)

    if (url.scheme() != scheme())
        return false;

    if (role == kItemFilePathRole) {
        QSharedPointer<RecentFileInfo> info = InfoFactory::create<RecentFileInfo>(url);
        if (info) {
            data->setValue(info->redirectedFileUrl().path());
            return true;
        }
    }

    if (role == kItemFileLastReadRole) {
        QSharedPointer<RecentFileInfo> info = InfoFactory::create<RecentFileInfo>(url);
        if (info) {
            data->setValue(info->lastRead().toString(FileUtils::dateTimeFormat()));
            return true;
        }
    }

    return false;
}

bool RecentManager::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (url == rootUrl()) {
        *iconName = SystemPathUtil::instance()->systemPathIconName("Recent");
        if (!iconName->isEmpty())
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

    watcher = WatcherFactory::create<AbstractFileWatcher>(QUrl::fromLocalFile(RecentManager::xbelPath()));
    connect(watcher.data(), &AbstractFileWatcher::subfileCreated, this, &RecentManager::updateRecent);
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentManager::updateRecent);
    watcher->startWatcher();

    connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &RecentManager::updateRecent);
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

DSB_FM_NAMESPACE::WindowsService *RecentManager::winServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WindowsService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WindowsService>(DSB_FM_NAMESPACE::WindowsService::name());
}

DSB_FM_NAMESPACE::TitleBarService *RecentManager::titleServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::TitleBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::TitleBarService>(DSB_FM_NAMESPACE::TitleBarService::name());
}

SideBarService *RecentManager::sideBarServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::SideBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::SideBarService>(DSB_FM_NAMESPACE::SideBarService::name());
}

DSB_FM_NAMESPACE::WorkspaceService *RecentManager::workspaceServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WorkspaceService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WorkspaceService>(DSB_FM_NAMESPACE::WorkspaceService::name());
}

dfm_service_common::FileOperationsService *RecentManager::fileOperationsServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::FileOperationsService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
}

EventSequenceManager *RecentManager::eventSequence()
{
    return &dpfInstance.eventSequence();
}
