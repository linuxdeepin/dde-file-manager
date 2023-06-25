// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bookmark.h"
#include "utils/bookmarkhelper.h"
#include "controller/bookmarkmanager.h"
#include "controller/defaultitemmanager.h"
#include "events/bookmarkeventreceiver.h"
#include "menu/bookmarkmenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE

void BookMark::initialize()
{
    bindEvents();
    followEvents();
    bindWindows();
}

bool BookMark::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(BookmarkMenuCreator::name(), new BookmarkMenuCreator);
    bindScene("FileOperatorMenu");

    return true;
}

void BookMark::onWindowOpened(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");

    if (window->sideBar()) {
        onSideBarInstallFinished();
    } else {
        connect(window, &FileManagerWindow::sideBarInstallFinished, this,
                &BookMark::onSideBarInstallFinished, Qt::DirectConnection);
    }
}

void BookMark::onSideBarInstallFinished()
{
    // Workaround: Each process only needs to initialize the bookmark data once
    static std::once_flag flag;
    std::call_once(flag, []() {
        DefaultItemManager::instance()->initDefaultItems();
        BookMarkManager::instance()->addQuickAccessItemsFromConfig();
    });
}

void BookMark::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(BookmarkMenuCreator::name(), parentScene);
    } else {
        menuScenes << parentScene;
        if (!subscribedEvent)
            subscribedEvent = dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &BookMark::onMenuSceneAdded);
    }
}

void BookMark::onMenuSceneAdded(const QString &scene)
{
    if (menuScenes.contains(scene)) {
        menuScenes.remove(scene);
        dfmplugin_menu_util::menuSceneBind(BookmarkMenuCreator::name(), scene);

        if (menuScenes.isEmpty()) {
            dpfSignalDispatcher->unsubscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded", this, &BookMark::onMenuSceneAdded);
            subscribedEvent = false;
        }
    }
}

void BookMark::bindEvents()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult,
                                   BookMarkEventReceiver::instance(),
                                   &BookMarkEventReceiver::handleRenameFile);
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_Sidebar_Sorted",
                                   BookMarkEventReceiver::instance(), &BookMarkEventReceiver::handleSidebarOrderChanged);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPBOOKMARK_NAMESPACE), "slot_Scheme_Disable",
                            BookMarkEventReceiver::instance(), &BookMarkEventReceiver::handleAddSchemeOfBookMarkDisabled);
    dpfSlotChannel->connect(DPF_MACRO_TO_STR(DPBOOKMARK_NAMESPACE), "slot_AddPluginItem",
                            BookMarkEventReceiver::instance(), &BookMarkEventReceiver::handlePluginItem);
}

void BookMark::bindWindows()
{
    const auto &winIdList { FMWindowsIns.windowIdList() };
    std::for_each(winIdList.begin(), winIdList.end(), [this](quint64 id) {
        onWindowOpened(id);
    });
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this,
            &BookMark::onWindowOpened, Qt::DirectConnection);
}

void BookMark::followEvents()
{
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Group_Sort", BookMarkEventReceiver::instance(), &BookMarkEventReceiver::handleItemSort);
}
