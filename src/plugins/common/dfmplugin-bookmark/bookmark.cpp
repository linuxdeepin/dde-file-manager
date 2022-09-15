/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "bookmark.h"
#include "utils/bookmarkhelper.h"
#include "controller/bookmarkmanager.h"
#include "controller/defaultitemmanager.h"
#include "events/bookmarkeventreceiver.h"
#include "menu/bookmarkmenuscene.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE

void BookMark::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this,
            &BookMark::onWindowCreated, Qt::DirectConnection);

    bindEvents();
    followEvents();
}

bool BookMark::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(BookmarkMenuCreator::name(), new BookmarkMenuCreator);
    bindScene("FileOperatorMenu");

    return true;
}

void BookMark::onWindowCreated(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");

    connect(window, &FileManagerWindow::sideBarInstallFinished, this,
            []() {
                BookMarkManager::instance()->initDefaultItems();
                BookMarkManager::instance()->addBookMarkItemsFromConfig();
            },
            Qt::DirectConnection);
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

void BookMark::followEvents()
{
    dpfHookSequence->follow("dfmplugin_sidebar", "hook_Group_Sort", BookMarkEventReceiver::instance(), &BookMarkEventReceiver::handleItemSort);
}
