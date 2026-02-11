// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentmanager.h"
#include "files/recentfileinfo.h"
#include "files/recentfilewatcher.h"
#include "utils/recentfilehelper.h"
#include "events/recenteventcaller.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/dfileinfo.h>

#include <DDialog>
#include <DRecentManager>
#include <DDesktopServices>

#include <QFile>
#include <QMenu>
#include <QApplication>

#include <mutex>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace dfmplugin_recent;
DPF_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace GlobalServerDefines;

RecentManager *RecentManager::instance()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    // data race
    static RecentManager instance;
    return &instance;
}

RecentManagerDBusInterface *RecentManager::dbus() const
{
    return recentDBusInterce.data();
}

QMap<QUrl, FileInfoPointer> RecentManager::getRecentNodes() const
{
    QMap<QUrl, FileInfoPointer> nodes;
    for (auto it = recentItems.constBegin(); it != recentItems.constEnd(); ++it)
        nodes.insert(it.key(), it.value().fileInfo);

    return nodes;
}

QString RecentManager::getRecentOriginPaths(const QUrl &url) const
{
    auto it = recentItems.find(url);
    if (it != recentItems.end())
        return it.value().originPath;

    fmDebug() << "No origin path found for URL:" << url;
    return QString();
}

RecentManager::RecentManager(QObject *parent)
    : QObject(parent)
{
}

RecentManager::~RecentManager()
{
}

void RecentManager::resetRecentNodes()
{
    auto reply = recentDBusInterce->GetItemsInfo();
    reply.waitForFinished();
    const QVariantList &topLevelList = reply.value();
    for (const auto &value : topLevelList) {
        QDBusArgument dbusArg = value.value<QDBusArgument>();

        QVariantMap map;
        dbusArg >> map;   // 直接将QDBusArgument解包为QVariantMap

        if (!map.isEmpty()) {
            auto path = map.value(RecentProperty::kPath).toString();
            auto href = map.value(RecentProperty::kHref).toString();
            auto modified = map.value(RecentProperty::kModified).toLongLong();
            onItemAdded(path, href, modified);
        } else {
            fmWarning() << "Map is empty or could not be converted from DBus argument";
        }
    }
}

bool RecentManager::removeRecentFile(const QUrl &url)
{
    return recentItems.remove(url) > 0;
}

int RecentManager::size()
{
    return recentItems.size();
}

void RecentManager::init()
{
    recentDBusInterce.reset(
            new RecentManagerDBusInterface("org.deepin.Filemanager.Daemon",
                                           "/org/deepin/Filemanager/Daemon/RecentManager",
                                           QDBusConnection::sessionBus(), this));
    recentDBusInterce->setTimeout(2000);

    connect(recentDBusInterce.data(), &RecentManagerDBusInterface::ReloadFinished,
            this, [this](qint64 timestamp) {
                fmDebug() << "DBus reload finished, timestamp:" << timestamp;
                if (timestamp != 0)
                    resetRecentNodes();
                static std::once_flag flag;
                std::call_once(flag, [this]() {
                    // 初始化的过程中可能会发送大量信号
                    connect(recentDBusInterce.data(), &RecentManagerDBusInterface::ItemAdded, this, &RecentManager::onItemAdded);
                    connect(recentDBusInterce.data(), &RecentManagerDBusInterface::ItemsRemoved, this, &RecentManager::onItemsRemoved);
                    connect(recentDBusInterce.data(), &RecentManagerDBusInterface::ItemChanged, this, &RecentManager::onItemChanged);
                });
            });

    auto reply = recentDBusInterce->Reload();
    reply.waitForFinished();

    connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &RecentManager::reloadRecent);
}

void RecentManager::reloadRecent()
{
    fmDebug() << "Reloading recent files from DBus service";
    recentDBusInterce->Reload();
}

void RecentManager::onItemAdded(const QString &path, const QString &href, qint64 modified)
{
    if (path.isEmpty()) {
        fmWarning() << "Item added with empty path, ignoring";
        return;
    }

    const QUrl &url { RecentHelper::recentUrl(path) };
    if (!url.isValid()) {
        fmWarning() << "Add node failed, invalid URL for path:" << path;
        return;
    }

    if (recentItems.contains(url))
        return;

    auto info = InfoFactory::create<FileInfo>(url);
    if (info.isNull()) {
        fmWarning() << "Add node failed, could not create FileInfo for URL:" << url;
        return;
    }

    fmDebug() << "Adding recent item to cache:" << url;
    RecentItem item;
    item.fileInfo = info;
    item.originPath = href;
    recentItems.insert(url, item);
    item.fileInfo->cacheAttribute(DFMIO::DFileInfo::AttributeID::kTimeAccess, modified);

    QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentHelper::rootUrl());
    if (watcher)
        emit watcher->subfileCreated(url);
}

void RecentManager::onItemsRemoved(const QStringList &paths)
{
    for (const auto &path : paths) {
        const QUrl &url { RecentHelper::recentUrl(path) };
        if (!recentItems.contains(url)) {
            fmDebug() << "Item not found in cache for removal:" << url;
            continue;
        }

        fmDebug() << "Removing recent item from cache:" << url;
        recentItems.remove(url);
        QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentHelper::rootUrl());
        if (watcher)
            emit watcher->fileDeleted(url);
    }
}

void RecentManager::onItemChanged(const QString &path, qint64 modified)
{
    if (path.isEmpty()) {
        fmWarning() << "Item changed with empty path, ignoring";
        return;
    }

    const QUrl &url { RecentHelper::recentUrl(path) };
    if (!recentItems.contains(url)) {
        fmDebug() << "Item not found in cache for update:" << url;
        return;
    }

    fmDebug() << "Updating recent item access time - path:" << path << "timestamp:" << modified;
    QDateTime dateTime { QDateTime::fromSecsSinceEpoch(modified) };
    recentItems[url].fileInfo->cacheAttribute(DFMIO::DFileInfo::AttributeID::kTimeAccess, modified);

    QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(RecentHelper::rootUrl());
    if (watcher)
        emit watcher->fileAttributeChanged(url);
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
    // In wayland , dialog needs to set a parent , otherwise it will enter the window modal incorrectly
    DDialog dlg(qApp->activeWindow());
    dlg.setIcon(QIcon::fromTheme("dde-file-manager"));
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
            QString href = RecentManager::instance()->getRecentOriginPaths(url);
            if (!href.isEmpty()) {
                list << href;
                continue;
            }
            // list << DUrl::fromLocalFile(url.path()).toString();
            // 通过durl转换path会出现编码问题，这里直接用字符串拼出正确的path;
            QUrl newUrl = url;
            newUrl.setScheme(Global::Scheme::kFile);
            list << newUrl.toString();
        }

        if (list.size() == RecentManager::instance()->size())
            RecentManager::instance()->dbus()->PurgeItems();   // 此时为全选，purge性能更高
        else
            RecentManager::instance()->dbus()->RemoveItems(list);
    }
}

void RecentHelper::clearRecent()
{
    RecentManager::instance()->dbus()->PurgeItems();
}

bool RecentHelper::openFileLocation(const QUrl &url)
{
    QUrl localUrl = url;
    QList<QUrl> urls {};
    bool ok = UniversalUtils::urlsTransformToLocal({ localUrl }, &urls);
    if (ok && !urls.isEmpty())
        localUrl = urls.first();

    if (SysInfoUtils::isRootUser()) {
        QStringList urls { url.toString() };
        return QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << urls << "--raw");
    }

    return DDesktopServices::showFileItem(localUrl);
}

void RecentHelper::openFileLocation(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        if (!openFileLocation(url))
            fmWarning() << "Failed to open file location for:" << url.path();
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
        fmDebug() << "Context menu action executed:" << act->text();
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

QUrl RecentHelper::recentUrl(const QString &path)
{
    QUrl recentUrl { QUrl::fromLocalFile(path) };
    recentUrl.setScheme(RecentHelper::scheme());
    return recentUrl;
}
