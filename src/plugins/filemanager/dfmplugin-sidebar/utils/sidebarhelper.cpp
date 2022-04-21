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
#include "services/filemanager/workspace/workspaceservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/systempathutil.h"

#include <dfm-framework/framework.h>

#include <QMenu>

DPSIDEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QMap<quint64, SideBarWidget *> SideBarHelper::kSideBarMap {};
QMap<QString, SideBar::SortFunc> SideBarHelper::kSortFuncs {};

QList<SideBarWidget *> SideBarHelper::allSideBar()
{
    QMutexLocker locker(&SideBarHelper::mutex());
    QList<SideBarWidget *> list;
    auto keys = kSideBarMap.keys();
    for (auto k : keys)
        list.push_back(kSideBarMap[k]);

    return list;
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
    auto flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDropEnabled };
    DSB_FM_NAMESPACE::SideBar::ItemInfo info;
    info.group = group;
    info.iconName = iconName;
    info.text = text;
    info.url = url;
    info.flags = flags;
    info.cdCb = defaultCdAction;
    info.contextMenuCb = defaultContenxtMenu;
    item->setItemInfo(info);

    item->setFlags(flags);

    return item;
}

SideBarItem *SideBarHelper::createItemByInfo(const SideBar::ItemInfo &info)
{
    SideBarItem *item = new SideBarItem(QIcon::fromTheme(info.iconName),
                                        info.text,
                                        info.group,
                                        info.url);
    item->setFlags(info.flags);
    item->setItemInfo(info);

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
    auto &ctx = dpfInstance.serviceContext();
    auto workspaceService = ctx.service<WorkspaceService>(WorkspaceService::name());
    if (!workspaceService) {
        qCritical() << "Failed, defaultContenxtMenu \"WorkspaceService\" is empty";
        abort();
    }
    newTabAct->setDisabled(!workspaceService->tabAddable(windowId));

    menu->addSeparator();
    menu->addAction(QObject::tr("Properties"), [url]() {
        SideBarEventCaller::sendShowFilePropertyDialog(url);
    });
    menu->exec(globalPos);
    delete menu;
}

bool SideBarHelper::registerSortFunc(const QString &subGroup, SideBar::SortFunc func)
{
    if (kSortFuncs.contains(subGroup)) {
        qDebug() << subGroup << "has already been registered";
        return false;
    }
    kSortFuncs.insert(subGroup, func);
    return true;
}

SideBar::SortFunc SideBarHelper::sortFunc(const QString &subGroup)
{
    return kSortFuncs.value(subGroup, nullptr);
}

QMutex &SideBarHelper::mutex()
{
    static QMutex m;
    return m;
}
