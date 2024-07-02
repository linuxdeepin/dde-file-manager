// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentmanager.h"
#include "files/recentfileinfo.h"
#include "files/recentfilewatcher.h"
#include "utils/recentfilehelper.h"
#include "events/recenteventcaller.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>

#include <DDialog>
#include <DRecentManager>

#include <QFile>
#include <QMenu>
#include <QCoreApplication>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_recent;
DPF_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static constexpr char kEmptyRecentFile[] =
        R"|(<?xml version="1.0" encoding="UTF-8"?>
        <xbel version="1.0"
        xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks"
        xmlns:mime="http://www.freedesktop.org/standards/shared-mime-info"
        >
        </xbel>)|";

RecentManager *RecentManager::instance()
{
    // data race
    static RecentManager instance;
    return &instance;
}

QMap<QUrl, FileInfoPointer> RecentManager::getRecentNodes() const
{
    return recentNodes.map();
}

QMap<QUrl, QString> RecentManager::getRecentOriginPaths() const
{
    return recentOriginPaths;
}

bool RecentManager::removeRecentFile(const QUrl &url)
{
    if (recentNodes.contains(url)) {
        recentNodes.remove(url);
        recentOriginPaths.remove(url);
        return true;
    }
    return false;
}

bool RecentManager::customColumnRole(const QUrl &rootUrl, QList<ItemRoles> *roleList)
{
    if (rootUrl.scheme() == RecentHelper::scheme()) {
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
    if (url.scheme() != RecentHelper::scheme())
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

bool RecentManager::detailViewIcon(const QUrl &url, QString *iconName)
{
    if (url == RecentHelper::rootUrl()) {
        *iconName = SystemPathUtil::instance()->systemPathIconName("Recent");
        if (!iconName->isEmpty())
            return true;
    }
    return false;
}

bool RecentManager::sepateTitlebarCrumb(const QUrl &url, QList<QVariantMap> *mapGroup)
{
    Q_ASSERT(mapGroup);

    if (url.scheme() == RecentHelper::scheme()) {
        QVariantMap map;
        map["CrumbData_Key_Url"] = RecentHelper::rootUrl();
        map["CrumbData_Key_DisplayText"] = tr("Recent");
        map["CrumbData_Key_IconName"] = RecentHelper::icon().name();
        mapGroup->push_back(map);
        return true;
    }

    return false;
}

bool RecentManager::isTransparent(const QUrl &url, TransparentStatus *status)
{
    if (url.scheme() == RecentHelper::scheme()) {
        *status = TransparentStatus::kUntransparent;
        return true;
    }

    return false;
}

bool RecentManager::checkDragDropAction(const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action)
{
    Q_UNUSED(urlTo)
    Q_ASSERT(action);

    if (urls.isEmpty())
        return false;

    if (urls.first().scheme() == RecentHelper::scheme()) {
        *action = Qt::CopyAction;
        return true;
    }

    return false;
}

bool RecentManager::handleDropFiles(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    if (fromUrls.isEmpty() || !toUrl.isValid())
        return false;

    if (fromUrls.first().scheme() == RecentHelper::scheme() && toUrl.scheme() == "trash") {
        RecentHelper::removeRecent(fromUrls);
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
}

void RecentManager::init()
{
    iteratorWorker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, iteratorWorker, &QObject::deleteLater);
    connect(this, &RecentManager::asyncHandleFileChanged,
            iteratorWorker, &RecentIterateWorker::onRecentFileChanged);

    connect(iteratorWorker, &RecentIterateWorker::updateRecentFileInfo, this,
            &RecentManager::onUpdateRecentFileInfo);
    connect(iteratorWorker, &RecentIterateWorker::deleteExistRecentUrls, this,
            &RecentManager::onDeleteExistRecentUrls);
    connect(qApp, &QApplication::aboutToQuit, this, [this](){
        if (watcher) {
            watcher->stopWatcher();
            watcher->disconnect(this);
        }
        iteratorWorker->stop();
        if (workerThread.isRunning()) {
            workerThread.quit();
            workerThread.wait();
        }
    });

    workerThread.start();

    emit asyncHandleFileChanged({});

    watcher = WatcherFactory::create<AbstractFileWatcher>(QUrl::fromLocalFile(RecentHelper::xbelPath()));
    connect(watcher.data(), &AbstractFileWatcher::subfileCreated, this, &RecentManager::updateRecent);
    connect(watcher.data(), &AbstractFileWatcher::fileAttributeChanged, this, &RecentManager::updateRecent);
    watcher->startWatcher();

    connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &RecentManager::updateRecent);
}

void RecentManager::updateRecent()
{
    emit asyncHandleFileChanged(recentNodes.keys());
}

void RecentManager::onUpdateRecentFileInfo(const QUrl &url, const QString &originPath, qint64 readTime)
{
    if (!recentNodes.contains(url)) {
        recentNodes.insert(url, InfoFactory::create<FileInfo>(url));
        recentOriginPaths[url] = originPath;
        QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentHelper::rootUrl());
        if (watcher) {
            emit watcher->subfileCreated(url);
        }
    }

    // ToDo(yanghao):update read time
    Q_UNUSED(readTime)
}

void RecentManager::onDeleteExistRecentUrls(const QList<QUrl> &urls)
{
    for (const auto &url : urls) {
        if (removeRecentFile(url)) {
            QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentHelper::rootUrl());
            if (watcher) {
                emit watcher->fileDeleted(url);
            }
        }
    }
}

QUrl RecentHelper::rootUrl()
{
    QUrl url;
    url.setScheme(RecentHelper::scheme());
    url.setPath("/");
    url.setHost("");
    return url;
}

void RecentHelper::removeRecent(const QList<QUrl> &urls)
{
    //In wayland , dialog needs to set a parent , otherwise it will enter the window modal incorrectly
    DDialog dlg(qApp->activeWindow());
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
        auto originPath = RecentManager::instance()->getRecentOriginPaths();
        for (const QUrl &url : urls) {
            if (originPath.contains(url)) {
                list << originPath[url];
                continue;
            }
            // list << DUrl::fromLocalFile(url.path()).toString();
            // 通过durl转换path会出现编码问题，这里直接用字符串拼出正确的path;
            QUrl newUrl = url;
            newUrl.setScheme(Global::Scheme::kFile);
            list << newUrl.toString();
        }

        DRecentManager::removeItems(list);
    }
}

void RecentHelper::clearRecent()
{
    QFile f(RecentHelper::xbelPath());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(kEmptyRecentFile);
        f.close();
    } else {
        fmWarning() << "open recent xbel file failed!!!";
    }
}

bool RecentHelper::openFileLocation(const QUrl &url)
{
    QUrl localUrl = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    const auto &fileInfo { InfoFactory::create<FileInfo>(localUrl) };
    QUrl parentUrl { fileInfo->urlOf(UrlInfoType::kParentUrl) };
    parentUrl.setQuery("selectUrl=" + localUrl.toString());

    return dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, parentUrl);
}

void RecentHelper::openFileLocation(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        if (!openFileLocation(url))
            fmWarning() << "failed to open: " << url.path();
    }
}

void RecentHelper::contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos)
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
        RecentHelper::clearRecent();
    });
    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_recent", "signal_ReportLog_MenuData", act->text(), urls);
    }
    delete menu;
}

ExpandFieldMap RecentHelper::propetyExtensionFunc(const QUrl &url)
{
    BasicExpand expand;
    const auto &info = InfoFactory::create<FileInfo>(url);
    const QString &sourcePath = info->urlOf(UrlInfoType::kRedirectedFileUrl).toLocalFile();
    expand.insert("kFileModifiedTime", qMakePair(QObject::tr("Source path"), sourcePath));

    ExpandFieldMap map;
    map["kFieldInsert"] = expand;

    return map;
}

QUrl RecentHelper::urlTransform(const QUrl &url)
{
    if (UrlRoute::isRootUrl(url))
        return url;

    QUrl out { url };
    out.setScheme(Global::Scheme::kFile);
    return out;
}
