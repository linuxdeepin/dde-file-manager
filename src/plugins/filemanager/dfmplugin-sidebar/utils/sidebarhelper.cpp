/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "sidebarhelper.h"
#include "views/sidebaritem.h"
#include "events/sidebareventcaller.h"

#include "services/filemanager/windows/windowsservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/systempathutil.h"

#include <dfm-framework/framework.h>

#include <QMenu>

DPSIDEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QMap<quint64, SideBarWidget *> SideBarHelper::kSideBarMap {};
QList<DSB_FM_NAMESPACE::SideBar::ItemInfo> SideBarHelper::kCacheInfo {};

QList<SideBarWidget *> SideBarHelper::allSideBar()
{
    QMutexLocker locker(&SideBarHelper::mutex());
    QList<SideBarWidget *> list;
    auto keys = kSideBarMap.keys();
    for (auto k : keys)
        list.push_back(kSideBarMap[k]);

    return list;
}

QList<SideBar::ItemInfo> SideBarHelper::allCacheInfo()
{
    return kCacheInfo;
}

void SideBarHelper::removeItemFromCache(const QUrl &url)
{
    for (int i = 0; i < kCacheInfo.size(); i++) {
        if (kCacheInfo.at(i).url == url) {
            kCacheInfo.removeAt(i);
            return;
        }
    }
}

SideBarWidget *SideBarHelper::findSideBarByWindowId(quint64 windowId)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (!kSideBarMap.contains(windowId))
        return nullptr;

    return kSideBarMap[windowId];
}

void SideBarHelper::addSideBar(quint64 windowId, SideBarWidget *titleBar)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (!kSideBarMap.contains(windowId))
        kSideBarMap.insert(windowId, titleBar);
}

void SideBarHelper::removeSideBar(quint64 windowId)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (kSideBarMap.contains(windowId))
        kSideBarMap.remove(windowId);
}

quint64 SideBarHelper::windowId(QWidget *sender)
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    return windowService->findWindowId(sender);
}

SideBarItem *SideBarHelper::createDefaultItem(const QString &pathKey, const QString &group)
{
    QString iconName { SystemPathUtil::instance()->systemPathIconName(pathKey) };
    QString text { SystemPathUtil::instance()->systemPathDisplayName(pathKey) };
    QString path { SystemPathUtil::instance()->systemPath(pathKey) };
    if (!iconName.contains("-symbolic"))
        iconName.append("-symbolic");

    QUrl url { UrlRoute::pathToReal(path) };
    SideBarItem *item = new SideBarItem(QIcon::fromTheme(iconName),
                                        text,
                                        group,
                                        url);

    item->setRegisteredHandler(makeItemIdentifier(group, url));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDropEnabled);

    return item;
}

SideBarItem *SideBarHelper::createItemByInfo(const SideBar::ItemInfo &info)
{
    SideBarItem *item = new SideBarItem(QIcon::fromTheme(info.iconName),
                                        info.text,
                                        info.group,
                                        info.url);
    item->setRegisteredHandler(makeItemIdentifier(info.group, info.url));
    item->setFlags(info.flag);

    // create `unmount action` for removable device
    if (info.removable) {
        DViewItemActionList lst;
        DViewItemAction *action = new DViewItemAction(Qt::AlignCenter, QSize(16, 16), QSize(), true);
        action->setIcon(QIcon::fromTheme("media-eject-symbolic"));
        action->setVisible(true);
        QObject::connect(action, &QAction::triggered, [info]() {
            SideBarEventCaller::sendEject(info.url);
        });
        lst.push_back(action);
        item->setActionList(Qt::RightEdge, lst);
    }

    if (!kCacheInfo.contains(info))
        kCacheInfo.push_back(info);

    return item;
}

SideBarItemSeparator *SideBarHelper::createSeparatorItem(const QString &group)
{
    SideBarItemSeparator *item = new SideBarItemSeparator(group);
    item->setFlags(Qt::NoItemFlags);
    return item;
}

QString SideBarHelper::makeItemIdentifier(const QString &group, const QUrl &url)
{
    return group + url.url();
}

void SideBarHelper::defaultCdAction(quint64 windowId, const QUrl &url)
{
    if (!url.isEmpty())
        SideBarEventCaller::sendItemActived(windowId, url);
}

void SideBarHelper::defaultContenxtMenu(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    // ref (DFMSideBarDefaultItemHandler::contextMenu)
    QMenu *menu = new QMenu;
    menu->addAction(QObject::tr("Open in new window"), [url]() {
        SideBarEventCaller::sendOpenWindow(url);
    });

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        SideBarEventCaller::sendOpenTab(windowId, url);
    });
    newTabAct->setDisabled(false);   // TODO(zhangs): tabAddableByWinId

    menu->addSeparator();
    menu->addAction(QObject::tr("Properties"), [url]() {
        // TODO(zhangs): show property dialog
    });
    menu->exec(globalPos);
    delete menu;
}

QMutex &SideBarHelper::mutex()
{
    static QMutex m;
    return m;
}
