// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebar.h"

#include "sidebarwidget.h"
#include "sidebaritem.h"
#include "utils/sidebarhelper.h"
#include "events/sidebareventreceiver.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

namespace dfmplugin_sidebar {
DFM_LOG_REISGER_CATEGORY(DPSIDEBAR_NAMESPACE)

DFMBASE_USE_NAMESPACE

void SideBar::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &SideBar::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &SideBar::onWindowClosed, Qt::DirectConnection);
    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &SideBar::onConfigChanged, Qt::DirectConnection);

    SideBarEventReceiver::instance()->bindEvents();
}

bool SideBar::start()
{
    QString err;
    if (!DConfigManager::instance()->addConfig(ConfigInfos::kConfName, &err)) {
        fmDebug() << "register dconfig failed: " << err;
        return false;
    }

    SideBarHelper::initDefaultSettingPanel();
    SideBarHelper::registCustomSettingItem();

    dpfSignalDispatcher->installEventFilter(DFMBASE_NAMESPACE::GlobalEventType::kShowSettingDialog, this,
                                            &SideBar::onAboutToShowSettingDialog);

    return true;
}

void SideBar::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    auto sidebar = new SideBarWidget;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(sidebar), AcName::kAcDmSideBar);
#endif
    SideBarHelper::addSideBar(windId, sidebar);
    // just for first window
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        initPreDefineItems();
    });
    window->installSideBar(sidebar);
    sidebar->updateItemVisiable(SideBarHelper::hiddenRules());
}

void SideBar::onWindowClosed(quint64 winId)
{
    if (FMWindowsIns.windowIdList().count() == 1) {
        auto win = FMWindowsIns.findWindowById(FMWindowsIns.windowIdList().first());
        if (win) {
            SideBarWidget *sb = dynamic_cast<SideBarWidget *>(win->sideBar());
            if (sb)
                sb->saveStateWhenClose();
        }
    }

    SideBarHelper::removeSideBar(winId);
}

void SideBar::onConfigChanged(const QString &cfg, const QString &key)
{
    if (cfg != QString(ConfigInfos::kConfName))
        return;

    if (key == QString(ConfigInfos::kVisiableKey)) {
        for (const auto &id : FMWindowsIns.windowIdList()) {
            auto win = FMWindowsIns.findWindowById(id);
            if (win) {
                auto sb = dynamic_cast<SideBarWidget *>(win->sideBar());
                if (sb) {
                    sb->updateItemVisiable(SideBarHelper::hiddenRules());
                }
            }
        }
    }
#ifdef TREEVIEW
    if (key == QString(ConfigInfos::kGroupExpandedKey)) {
        if (FMWindowsIns.windowIdList().count() <= 0)
            return;
        auto win = FMWindowsIns.findWindowById(FMWindowsIns.windowIdList().first());
        if (win) {
            auto sb = dynamic_cast<SideBarWidget *>(win->sideBar());
            if (sb) {
                sb->updateItemVisiable(SideBarHelper::groupExpandRules());
            }
        }
    }
#endif
}

// always return false?
// Just trying to bind data, not really filtering it
bool SideBar::onAboutToShowSettingDialog(quint64 winId)
{
    auto win { FMWindowsIns.findWindowById(winId) };
    if (!win) {
        fmWarning() << "Invalid window id";
        return false;
    }

    // bind setting datas
    auto widget = dynamic_cast<SideBarWidget *>(win->sideBar());
    widget->resetSettingPanel();

    return false;
}

void SideBar::initPreDefineItems()
{
    const auto &preDefineProperties { SideBarHelper::preDefineItemProperties() };
    // add item later for sort
    QList<QVariantMap> propertiesAdded;

    for (auto begin = preDefineProperties.begin(); begin != preDefineProperties.end(); ++begin) {
        int pos { begin.value().first };
        const QVariantMap &property { begin.value().second };
        if (pos < 0) {
            propertiesAdded.append(property);
            continue;
        }

        QUrl url { property.value(PropertyKey::kUrl).toUrl() };
        SideBarEventReceiver::instance()->handleItemInsert(pos, url, property);
    }

    for (const auto &property : propertiesAdded) {
        QUrl url { property.value(PropertyKey::kUrl).toUrl() };
        SideBarEventReceiver::instance()->handleItemAdd(url, property);
    }
}
}   // namespace dfmplugin_sidebar
