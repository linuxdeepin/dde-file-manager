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

#include "services/filemanager/windows/windowsservice.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/systempathutil.h"

#include <dfm-framework/framework.h>

DPSIDEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QMap<quint64, SideBarWidget *> SideBarHelper::kSideBarMap {};
QList<DSB_FM_NAMESPACE::SideBar::ItemInfo> SideBarHelper::cacheInfo {};

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
    return cacheInfo;
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

    SideBarItem *item = new SideBarItem(QIcon::fromTheme(iconName),
                                        text,
                                        group,
                                        UrlRoute::pathToReal(path));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDropEnabled);

    return item;
}

SideBarItem *SideBarHelper::createItemByInfo(const SideBar::ItemInfo &info)
{
    SideBarItem *item = new SideBarItem(QIcon::fromTheme(info.iconName),
                                        info.text,
                                        info.group,
                                        info.url);
    item->setFlags(info.flag);
    if (!cacheInfo.contains(info))
        cacheInfo.push_back(info);
    return item;
}

SideBarItemSeparator *SideBarHelper::createSeparatorItem(const QString &group)
{
    SideBarItemSeparator *item = new SideBarItemSeparator(group);
    item->setFlags(Qt::NoItemFlags);
    return item;
}

QMutex &SideBarHelper::mutex()
{
    static QMutex m;
    return m;
}
