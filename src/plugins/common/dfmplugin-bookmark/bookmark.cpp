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
#include "events/bookmarkeventreceiver.h"
#include "menu/bookmarkmenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include "services/filemanager/bookmark/bookmark_defines.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

using namespace dfmplugin_bookmark;
DFMBASE_USE_NAMESPACE

void BookMark::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowCreated, this,
            &BookMark::onWindowCreated, Qt::DirectConnection);

    dpfSignalDispatcher->subscribe(GlobalEventType::kRenameFileResult,
                                   BookMarkEventReceiver::instance(),
                                   &BookMarkEventReceiver::handleRenameFile);
    dpfSignalDispatcher->subscribe("dfmplugin_sidebar", "signal_SidebarSorted", BookMarkEventReceiver::instance(), &BookMarkEventReceiver::handleSidebarOrderChanged);
    dpfSignalDispatcher->subscribe(DSB_FM_NAMESPACE::BookMark::EventType::kBookMarkDisabled,
                                   BookMarkEventReceiver::instance(),
                                   &BookMarkEventReceiver::handleAddSchemeOfBookMarkDisabled);
}

bool BookMark::start()
{
    dfmplugin_menu_util::menuSceneRegisterScene(BookmarkMenuCreator::name(), new BookmarkMenuCreator);
    bindScene("FileOperatorMenu");

    return true;
}

dpf::Plugin::ShutdownFlag BookMark::stop()
{
    return kSync;
}

void BookMark::onWindowCreated(quint64 winId)
{
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");

    connect(window, &FileManagerWindow::sideBarInstallFinished, this,
            []() { BookMarkManager::instance()->addBookMarkItemsFromConfig(); },
            Qt::DirectConnection);
}

void BookMark::bindScene(const QString &parentScene)
{
    if (dfmplugin_menu_util::menuSceneContains(parentScene)) {
        dfmplugin_menu_util::menuSceneBind(BookmarkMenuCreator::name(), parentScene);
    } else {
        //todo(lym) menu
//        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &scene) {
//            if (scene == parentScene)
//                MenuService::service()->bind(BookmarkMenuCreator::name(), scene);
//        },
//                Qt::DirectConnection);
    }
}
