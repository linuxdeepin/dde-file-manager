// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebar.h"

#include "sidebarwidget.h"
#include "sidebaritem.h"
#include "utils/sidebarhelper.h"
#include "events/sidebareventreceiver.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_sidebar;
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
        qDebug() << "register dconfig failed: " << err;
        return false;
    }
    SideBarHelper::bindSettings();

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
