/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "sidebarunicastreceiver.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarmanager.h"
#include "views/sidebarwidget.h"
#include "views/sidebaritem.h"

#include <dfm-framework/framework.h>

DPSIDEBAR_USE_NAMESPACE

#define STR1(s) #s
#define STR2(s) STR1(s)

using namespace DSB_FM_NAMESPACE::SideBar;

/*!
 * \brief topic is defined in SideBarService
 * \param func
 * \return
 */
inline QString topic(const QString &func)
{
    return QString(STR2(DSB_FM_NAMESPACE)) + "::" + func;
}

SideBarUnicastReceiver *SideBarUnicastReceiver::instance()
{
    static SideBarUnicastReceiver receiver;
    return &receiver;
}

void SideBarUnicastReceiver::connectService()
{
    dpfInstance.eventUnicast().connect(topic("SideBarService::addItem"), this, &SideBarUnicastReceiver::invokeAddItem);
    dpfInstance.eventUnicast().connect(topic("SideBarService::removeItem"), this, &SideBarUnicastReceiver::invokeRemoveItem);
    dpfInstance.eventUnicast().connect(topic("SideBarService::updateItem"), this, &SideBarUnicastReceiver::invokeUpdateItem);
}

void SideBarUnicastReceiver::invokeAddItem(const ItemInfo &info, CdActionCallback cdFunc, ContextMenuCallback menuFunc, RenameCallback renameFunc)
{
    if (SideBarHelper::allCacheInfo().contains(info)) {
        qWarning() << "Cannot add repeated info " << info.url;
        return;
    }
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar) {
        SideBarItem *item = SideBarHelper::createItemByInfo(info);
        if (item) {
            sidebar->addItem(item);
            SideBarManager::instance()->registerCallback(item->registeredHandler(), cdFunc, menuFunc, renameFunc);
            QUrl &&itemUrl = item->url();
            QUrl &&sidebarUrl = sidebar->currentUrl().url();
            if (itemUrl.scheme() == sidebarUrl.scheme() && itemUrl.path() == sidebarUrl.path())
                sidebar->setCurrentUrl(item->url());
        }
    }
}

void SideBarUnicastReceiver::invokeRemoveItem(const QUrl &url)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->removeItem(url);
    SideBarHelper::removeItemFromCache(url);
}

void SideBarUnicastReceiver::invokeUpdateItem(const QUrl &url, const QString &newName)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->updateItem(url, newName);
}

SideBarUnicastReceiver::SideBarUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}
