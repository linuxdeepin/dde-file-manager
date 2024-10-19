// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebar.h"
#include "utils/titlebarhelper.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"
#include "utils/searchhistroymanager.h"
#include "views/titlebarwidget.h"
#include "events/titlebareventreceiver.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

namespace dfmplugin_titlebar {
DFM_LOG_REISGER_CATEGORY(DPTITLEBAR_NAMESPACE)

void TitleBar::initialize()
{
    DFMBASE_USE_NAMESPACE

    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this, &TitleBar::onWindowCreated, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &TitleBar::onWindowOpened, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &TitleBar::onWindowClosed, Qt::DirectConnection);

    // event has been sended before the Window showed
    bindEvents();
}

bool TitleBar::start()
{
    DFMBASE_USE_NAMESPACE
    // file scheme for crumbar
    dpfSlotChannel->push("dfmplugin_titlebar", "slot_Custom_Register", QString(Global::Scheme::kFile), QVariantMap {});

    return true;
}

void TitleBar::onWindowCreated(quint64 windId)
{
    TitleBarWidget *titleWidget = new TitleBarWidget;
    titleWidget->initTabBar(windId);
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(titleWidget), AcName::kAcComputerTitleBar);
#endif
    TitleBarHelper::addTileBar(windId, titleWidget);
}

void TitleBar::onWindowOpened(quint64 windId)
{
    DFMBASE_USE_NAMESPACE

    auto window = FMWindowsIns.findWindowById(windId);
    Q_ASSERT_X(window, "SideBar", "Cannot find window by id");
    TitleBarWidget *titleBarWidget = TitleBarHelper::findTileBarByWindowId(windId);
    Q_ASSERT_X(titleBarWidget, "SideBar", "Cannot find titlebar widget by id");
    window->installTitleBar(titleBarWidget);
    TitleBarHelper::createSettingsMenu(windId);

    NavWidget *navWidget = titleBarWidget->navWidget();
    Q_ASSERT(navWidget);
    connect(window, &FileManagerWindow::reqBack, navWidget, &NavWidget::back);
    connect(window, &FileManagerWindow::reqForward, navWidget, &NavWidget::forward);
    // First window's tab created before first url changed in titlebar
    connect(window, &FileManagerWindow::workspaceInstallFinished, navWidget,
            &NavWidget::onNewWindowOpended);
    connect(window, &FileManagerWindow::reqSearchCtrlF, titleBarWidget, &TitleBarWidget::handleHotkeyCtrlF);
    connect(window, &FileManagerWindow::reqSearchCtrlL, titleBarWidget, &TitleBarWidget::handleHotkeyCtrlL);
    connect(window, &FileManagerWindow::reqTriggerActionByIndex, titleBarWidget, &TitleBarWidget::handleHotketSwitchViewMode);
    connect(window, &FileManagerWindow::reqActivateNextTab, titleBarWidget, &TitleBarWidget::handleHotketNextTab);
    connect(window, &FileManagerWindow::reqActivatePreviousTab, titleBarWidget, &TitleBarWidget::handleHotketPreviousTab);
    connect(window, &FileManagerWindow::reqCreateTab, titleBarWidget, &TitleBarWidget::handleHotketCreateNewTab);
    connect(window, &FileManagerWindow::reqCloseCurrentTab, titleBarWidget, &TitleBarWidget::handleHotketCloseCurrentTab);
    connect(window, &FileManagerWindow::reqActivateTabByIndex, titleBarWidget, &TitleBarWidget::handleHotketActivateTab);
    connect(window, &FileManagerWindow::windowSplitterWidthChanged, titleBarWidget, &TitleBarWidget::handleSplitterAnimation);
}

void TitleBar::onWindowClosed(quint64 windId)
{
    TitleBarHelper::removeTitleBar(windId);
}

void TitleBar::bindEvents()
{
    dpfSignalDispatcher->subscribe(DFMBASE_NAMESPACE::kSwitchViewMode,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleViewModeChanged);
    dpfSignalDispatcher->subscribe(DFMBASE_NAMESPACE::kOpenNewTab,
                                   TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleOpenNewTabTriggered);

    // bind self slot events  slot_Spinner_Start
    static constexpr auto curSpace { DPF_MACRO_TO_STR(DPTITLEBAR_NAMESPACE) };
    dpfSlotChannel->connect(curSpace, "slot_Custom_Register",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleCustomRegister);
    dpfSlotChannel->connect(curSpace, "slot_Spinner_Start",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleStartSpinner);
    dpfSlotChannel->connect(curSpace, "slot_Spinner_Stop",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleStopSpinner);
    dpfSlotChannel->connect(curSpace, "slot_FilterButton_Show",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleShowFilterButton);
    dpfSlotChannel->connect(curSpace, "slot_NewWindowAndTab_SetEnable",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleSetNewWindowAndTabEnable);
    dpfSlotChannel->connect(curSpace, "slot_Navigator_Backward",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleWindowBackward);
    dpfSlotChannel->connect(curSpace, "slot_Navigator_Forward",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleWindowForward);
    dpfSlotChannel->connect(curSpace, "slot_Navigator_Remove",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleRemoveHistory);
    dpfSlotChannel->connect(curSpace, "slot_ServerDialog_RemoveHistory",
                            SearchHistroyManager::instance(), &SearchHistroyManager::removeSearchHistory);
    dpfSlotChannel->connect(curSpace, "slot_Tab_Addable",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleTabAddable);
    dpfSlotChannel->connect(curSpace, "slot_Tab_Close",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleCloseTabs);
    dpfSlotChannel->connect(curSpace, "slot_Tab_SetAlias",
                            TitleBarEventReceiver::instance(), &TitleBarEventReceiver::handleSetTabAlias);
}

}   // namespace dfmplugin_titlebar
