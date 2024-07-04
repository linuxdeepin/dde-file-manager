// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebareventreceiver.h"

#include "sidebarwidget.h"
#include "sidebaritem.h"
#include "sidebarmodel.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>

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
    dpfSlotChannel->connect(kCurrentEventSpace, "slot_Sidebar_UpdateSelection", this, &SideBarEventReceiver::handleSidebarUpdateSelection);
}

void SideBarEventReceiver::handleItemHidden(const QUrl &url, bool visible)
{
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

void SideBarEventReceiver::handleSidebarUpdateSelection(quint64 winId)
{
    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    for (SideBarWidget *sidebar : allSideBar) {
        if (SideBarHelper::windowId(sidebar) == winId) {
            sidebar->updateSelection();
            break;
        }
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
        return wid->findItemUrlsByGroupName(group);

    fmDebug() << "cannot find sidebarwidget for winid: " << winId << group;
    return {};
}

bool SideBarEventReceiver::handleItemAdd(const QUrl &url, const QVariantMap &properties)
{
    // TODO(zhangs): use model direct
    ItemInfo info { url, properties };
    if (SideBarInfoCacheMananger::instance()->contains(info))
        return false;

    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    if (!allSideBar.isEmpty()) {
        SideBarInfoCacheMananger::instance()->addItemInfoCache(info);
        SideBarItem *item = SideBarHelper::createItemByInfo(info);
        auto sidebar = allSideBar.first();
        if (item) {
            // TODO(zhangs): refactor, register ?
            bool direct = item->group() == DefaultGroup::kDevice ? false : true;
            if (sidebar->addItem(item, direct) == -1)
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
    if (!SideBarInfoCacheMananger::instance()->contains(url))
        return false;
    SideBarInfoCacheMananger::instance()->removeItemInfoCache(url);
    if (SideBarWidget::kSidebarModelIns)
        return SideBarWidget::kSidebarModelIns->removeRow(url);
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
    if (properties.contains(PropertyKey::kVisiableControlKey))
        info.visiableControlKey = properties[PropertyKey::kVisiableControlKey].toString();
    if (properties.contains(PropertyKey::kVisiableDisplayName))
        info.visiableDisplayName = properties[PropertyKey::kVisiableDisplayName].toString();
    if (properties.contains(PropertyKey::kReportName))
        info.reportName = properties[PropertyKey::kReportName].toString();

    if (properties.contains(PropertyKey::kCallbackItemClicked))
        info.clickedCb = DPF_NAMESPACE::paramGenerator<ItemClickedActionCallback>(properties[PropertyKey::kCallbackItemClicked]);
    if (properties.contains(PropertyKey::kCallbackContextMenu))
        info.contextMenuCb = DPF_NAMESPACE::paramGenerator<ContextMenuCallback>(properties[PropertyKey::kCallbackContextMenu]);
    if (properties.contains(PropertyKey::kCallbackRename))
        info.renameCb = DPF_NAMESPACE::paramGenerator<RenameCallback>(properties[PropertyKey::kCallbackRename]);
    if (properties.contains(PropertyKey::kCallbackFindMe))
        info.findMeCb = DPF_NAMESPACE::paramGenerator<FindMeCallback>(properties[PropertyKey::kCallbackFindMe]);

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

    QList<SideBarWidget *> allSideBar = SideBarHelper::allSideBar();
    if (!allSideBar.isEmpty()) {
        SideBarInfoCacheMananger::instance()->insertItemInfoCache(index, info);
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
