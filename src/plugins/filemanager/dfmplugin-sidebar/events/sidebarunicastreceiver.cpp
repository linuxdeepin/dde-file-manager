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
#include "utils/sidebarinfocachemananger.h"
#include "views/sidebarwidget.h"
#include "views/sidebaritem.h"

#include "dfm-base/utils/universalutils.h"

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
    dpfInstance.eventUnicast().connect(topic("SideBarService::updateItemName"), this, &SideBarUnicastReceiver::invokeUpdateItemName);
    dpfInstance.eventUnicast().connect(topic("SideBarService::updateItemIcon"), this, &SideBarUnicastReceiver::invokeUpdateItemIcon);
    dpfInstance.eventUnicast().connect(topic("SideBarService::insertItem"), this, &SideBarUnicastReceiver::invokeInsertItem);
    dpfInstance.eventUnicast().connect(topic("SideBarService::triggerItemEdit"), this, &SideBarUnicastReceiver::invokeTriggerItemEdit);
    dpfInstance.eventUnicast().connect(topic("SideBarService::registerSortFunc"), this, &SideBarUnicastReceiver::invokeRegisterSortFunc);
}

void SideBarUnicastReceiver::invokeAddItem(const ItemInfo &info)
{
    if (SideBarInfoCacheMananger::instance()->contains(info))
        return;

    int ret { -1 };
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar) {
        SideBarItem *item = SideBarHelper::createItemByInfo(info);
        if (item) {
            ret = sidebar->addItem(item);
            // for select to computer
            QUrl &&itemUrl = item->url();
            QUrl &&sidebarUrl = sidebar->currentUrl();
            DFMBASE_USE_NAMESPACE
            if (UniversalUtils::urlEquals(itemUrl, sidebarUrl)
                || (info.targetUrl.isValid() && UniversalUtils::urlEquals(sidebarUrl, info.targetUrl)))
                sidebar->setCurrentUrl(item->url());
        }
    }

    if (ret != -1)
        SideBarInfoCacheMananger::instance()->addItemInfoCache(info);
}

void SideBarUnicastReceiver::invokeRemoveItem(const QUrl &url)
{
    SideBarInfoCacheMananger::instance()->removeItemInfoCache(url);
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->removeItem(url);
}

void SideBarUnicastReceiver::invokeUpdateItem(const QUrl &url, const ItemInfo &info)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->updateItem(url, info);
}

void SideBarUnicastReceiver::invokeUpdateItemName(const QUrl &url, const QString &newName, bool editable)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->updateItem(url, newName, editable);
}

void SideBarUnicastReceiver::invokeUpdateItemIcon(const QUrl &url, const QIcon &newIcon)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->updateItem(url, newIcon);
}

void SideBarUnicastReceiver::invokeInsertItem(int index, const ItemInfo &info)
{
    Q_ASSERT(index >= 0 && index <= UINT8_MAX);

    if (SideBarInfoCacheMananger::instance()->contains(info))
        return;

    bool ret { false };
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar) {
        SideBarItem *item = SideBarHelper::createItemByInfo(info);
        if (item) {
            ret = sidebar->insertItem(index, item);
            QUrl &&itemUrl = item->url();
            QUrl &&sidebarUrl = sidebar->currentUrl().url();
            if (itemUrl.scheme() == sidebarUrl.scheme() && itemUrl.path() == sidebarUrl.path())
                sidebar->setCurrentUrl(item->url());
        }
    }

    if (ret)
        SideBarInfoCacheMananger::instance()->insertItemInfoCache(index, info);
}

void SideBarUnicastReceiver::invokeTriggerItemEdit(quint64 winId, const QUrl &url)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar) {
        if (SideBarHelper::windowId(sidebar) == winId)
            sidebar->editItem(url);
    }
}

bool SideBarUnicastReceiver::invokeRegisterSortFunc(const QString &subGroup, SortFunc func)
{
    return SideBarHelper::registerSortFunc(subGroup, func);
}

SideBarUnicastReceiver::SideBarUnicastReceiver(QObject *parent)
    : QObject(parent)
{
}
