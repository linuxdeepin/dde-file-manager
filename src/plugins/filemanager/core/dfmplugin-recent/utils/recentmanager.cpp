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
#include "files/recentfilewatcher.h"
#include "events/recenteventcaller.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/base/device/deviceproxymanager.h"
#include "dfm-base/file/local/localfilehandler.h"

#include <dfm-io/dfmio_utils.h>

#include <DDialog>
#include <DRecentManager>

#include <QFile>
#include <QMenu>
#include <QCoreApplication>

using namespace dfmplugin_recent;
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

    newTabAct->setDisabled(!RecentEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();
    menu->addAction(QObject::tr("Clear recent history"), [url]() {
        RecentManager::clearRecent();
    });
    menu->exec(globalPos);
    delete menu;
}

RecentManager::ExpandFieldMap RecentManager::propetyExtensionFunc(const QUrl &url)
{
    BasicExpand expand;
    const auto &info = InfoFactory::create<AbstractFileInfo>(url);
    const QString &sourcePath = info->redirectedFileUrl().toLocalFile();
    expand.insert("kFileModifiedTime", qMakePair(QObject::tr("Source path"), sourcePath));

    ExpandFieldMap map;
    map["kFieldInsert"] = expand;

    return map;
}

QUrl RecentManager::urlTransform(const QUrl &url)
{
    if (UrlRoute::isRootUrl(url))
        return url;

    QUrl out { url };
    out.setScheme(Global::Scheme::kFile);
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
        roleList->append(kItemFileDisplayNameRole);
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

bool RecentManager::sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);

    if (url.scheme() == RecentManager::scheme()) {
        QVariantMap map;
        map["CrumbData_Key_Url"] = RecentManager::rootUrl();
        map["CrumbData_Key_DisplayText"] = tr("Recent");
        map["CrumbData_Key_IconName"] = RecentManager::icon().name();
        mapGroup->push_back(map);
        return true;
    }

    return false;
}

bool RecentManager::urlsToLocal(const QList<QUrl> &origins, QList<QUrl> *urls)
{
    if (!urls)
        return false;
    for (const QUrl &url : origins) {
        if (url.scheme() != RecentManager::scheme())
            return false;
        (*urls).push_back(urlTransform(url));
    }
    return true;
}

bool RecentManager::isTransparent(const QUrl &url, TransparentStatus *status)
{
    if (url.scheme() == RecentManager::scheme()) {
        *status = TransparentStatus::kUntransparent;
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
    iteratorWorker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, iteratorWorker.data(), &QObject::deleteLater);
    connect(this, &RecentManager::asyncHandleFileChanged, iteratorWorker.data(), &RecentIterateWorker::doWork);

    connect(iteratorWorker.data(), &RecentIterateWorker::updateRecentFileInfo, this,
            &RecentManager::onUpdateRecentFileInfo, Qt::BlockingQueuedConnection);
    connect(iteratorWorker.data(), &RecentIterateWorker::deleteExistRecentUrls, this,
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

void RecentManager::removeRecent(const QList<QUrl> &urls)
{
    DDialog dlg;
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(QObject::tr("Cancel", "button"));
    dlg.addButton(QObject::tr("Remove", "button"), true, DDialog::ButtonRecommend);

    if (urls.size() == 1)
        dlg.setTitle(QObject::tr("Do you want to remove this item?"));
    else
        dlg.setTitle(QObject::tr("Do yout want to remove %1 items?").arg(urls.size()));
    dlg.setMessage(QObject::tr("It does not delete the original files"));

    int code = dlg.exec();
    if (code == 1) {
        QStringList list;
        for (const QUrl &url : urls) {
            //list << DUrl::fromLocalFile(url.path()).toString();
            //通过durl转换path会出现编码问题，这里直接用字符串拼出正确的path;
            QUrl newUrl = url;
            newUrl.setScheme(Global::Scheme::kFile);
            list << newUrl.toString();
        }

        DTK_CORE_NAMESPACE::DRecentManager::removeItems(list);
    }
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
