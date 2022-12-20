/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "sidebareventreceiver.h"

#include "sidebarwidget.h"
#include "sidebaritem.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/dpf.h>

DPSIDEBAR_USE_NAMESPACE

SideBarEventReceiver *SideBarEventReceiver::instance()
{
    static SideBarEventReceiver ins;
    return &ins;
}

void SideBarEventReceiver::bindEvents()
{
    static constexpr char kCurrentEventSpace[] { DPF_MACRO_TO_STR(DPSIDEBAR_NAMESPACE) };

    dpfSlotChannel->connect(kCurrentEventSpace, "slot_ContextMenu_SetEnable", this, &SideBarEventReceiver::handleSetContextMenuEnable);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Group_UrlList", this, &SideBarEventReceiver::handleGetGroupItems);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Item_Add", this, &SideBarEventReceiver::handleItemAdd);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Item_Remove", this, &SideBarEventReceiver::handleItemRemove);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Item_Update", this, &SideBarEventReceiver::handleItemUpdate);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Item_Insert", this, &SideBarEventReceiver::handleItemInsert);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Item_Hidden", this, &SideBarEventReceiver::handleItemHidden);
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Item_TriggerEdit", this, &SideBarEventReceiver::handleItemTriggerEdit);
}

void SideBarEventReceiver::handleItemHidden(const QUrl &url, bool visible)
{
    if (visible)
        SideBarInfoCacheMananger::instance()->removeHiddenUrl(url);
    else
        SideBarInfoCacheMananger::instance()->addHiddenUrl(url);

    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar)
        sidebar->setItemVisiable(url, visible);
}

void SideBarEventReceiver::handleItemTriggerEdit(quint64 winId, const QUrl &url)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar) {
        if (SideBarHelper::windowId(sidebar) == winId)
            sidebar->editItem(url);
    }
}

void SideBarEventReceiver::handleSetContextMenuEnable(bool enable)
{
    SideBarHelper::contextMenuEnabled = enable;
}

QList<QUrl> SideBarEventReceiver::handleGetGroupItems(quint64 winId, const QString &group)
{
    if (group.isEmpty())
        return {};

    SideBarWidget *wid { nullptr };
    for (auto sb : SideBarHelper::allSideBar()) {
        if (FMWindowsIns.findWindowId(sb) == winId) {
            wid = sb;
            break;
        }
    }

    if (wid)
        return wid->findItems(group);

    qDebug() << "cannot find sidebarwidget for winid: " << winId << group;
    return {};
}

bool SideBarEventReceiver::handleItemAdd(const QUrl &url, const QVariantMap &properties)
{
    ItemInfo info { url, properties };
    if (SideBarInfoCacheMananger::instance()->contains(info))
        return false;

    SideBarInfoCacheMananger::instance()->addItemInfoCache(info);

    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    if (!allSideBar.isEmpty()) {
        SideBarItem *item = SideBarHelper::createItemByInfo(info);
        auto sidebar = allSideBar.first();
        if (item) {
            if (sidebar->addItem(item) == -1)
                return false;
            // for select to computer
            QUrl &&itemUrl = item->url();
            QUrl &&sidebarUrl = sidebar->currentUrl();
            DFMBASE_USE_NAMESPACE
            if (UniversalUtils::urlEquals(itemUrl, sidebarUrl)
                || (info.finalUrl.isValid() && UniversalUtils::urlEquals(sidebarUrl, info.finalUrl)))
                sidebar->setCurrentUrl(item->url());
            return true;
        }
    }

    return true;
}

bool SideBarEventReceiver::handleItemRemove(const QUrl &url)
{
    SideBarInfoCacheMananger::instance()->removeItemInfoCache(url);
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    if (!allSideBar.isEmpty())
        return allSideBar.first()->removeItem(url);

    return false;
}

bool SideBarEventReceiver::handleItemUpdate(const QUrl &url, const QVariantMap &properties)
{
    if (!SideBarInfoCacheMananger::instance()->contains(url))
        return false;

    ItemInfo info { SideBarInfoCacheMananger::instance()->itemInfo(url) };

    bool urlUpdated { false };
    if (properties.contains(PropertyKey::kUrl)) {
        auto &&newUrl { properties[PropertyKey::kUrl].toUrl() };
        if (!DFMBASE_NAMESPACE::UniversalUtils::urlEquals(newUrl, info.url)) {
            urlUpdated = true;
            info.url = newUrl;
            SideBarInfoCacheMananger::instance()->removeItemInfoCache(url);
        }
    }

    if (properties.contains(PropertyKey::kGroup))
        info.group = properties[PropertyKey::kGroup].toString();
    if (properties.contains(PropertyKey::kSubGroup))
        info.subGroup = properties[PropertyKey::kSubGroup].toString();
    if (properties.contains(PropertyKey::kDisplayName))
        info.displayName = properties[PropertyKey::kDisplayName].toString();
    if (properties.contains(PropertyKey::kIcon))
        info.icon = qvariant_cast<QIcon>(properties[PropertyKey::kIcon]);
    if (properties.contains(PropertyKey::kFinalUrl))
        info.finalUrl = properties[PropertyKey::kFinalUrl].toUrl();
    if (properties.contains(PropertyKey::kQtItemFlags))
        info.flags = qvariant_cast<Qt::ItemFlags>(properties[PropertyKey::kQtItemFlags]);
    if (properties.contains(PropertyKey::kIsEjectable))
        info.isEjectable = properties[PropertyKey::kIsEjectable].toBool();
    if (properties.contains(PropertyKey::kIsEditable))
        info.isEditable = properties[PropertyKey::kIsEditable].toBool();
    if (properties.contains(PropertyKey::kIsHidden))
        info.isHidden = properties[PropertyKey::kIsHidden].toBool();

    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    if (!allSideBar.isEmpty()) {
        bool ret { false };
        if (urlUpdated)
            ret = SideBarInfoCacheMananger::instance()->addItemInfoCache(info);
        else
            ret = SideBarInfoCacheMananger::instance()->updateItemInfoCache(url, info);
        allSideBar.first()->updateItem(url, info);
        return ret;
    }

    return false;
}

bool SideBarEventReceiver::handleItemInsert(int index, const QUrl &url, const QVariantMap &properties)
{
    Q_ASSERT(index >= 0 && index <= UINT8_MAX);

    ItemInfo info { url, properties };
    if (SideBarInfoCacheMananger::instance()->contains(info))
        return false;
    SideBarInfoCacheMananger::instance()->insertItemInfoCache(index, info);

    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    if (!allSideBar.isEmpty()) {
        SideBarItem *item = SideBarHelper::createItemByInfo(info);
        auto sidebar = allSideBar.first();
        if (item) {
            bool ret = sidebar->insertItem(index, item);
            QUrl &&itemUrl = item->url();
            QUrl &&sidebarUrl = sidebar->currentUrl().url();
            if (itemUrl.scheme() == sidebarUrl.scheme() && itemUrl.path() == sidebarUrl.path())
                sidebar->setCurrentUrl(item->url());
            return ret;
        }
    }

    return false;
}

SideBarEventReceiver::SideBarEventReceiver(QObject *parent)
    : QObject(parent)
{
}
