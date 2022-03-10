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

#include "dfm-base/dfm_event_defines.h"

#include "services/filemanager/bookmark/bookmark_defines.h"

DPBOOKMARK_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void BookMark::initialize()
{
    connect(BookMarkHelper::winServIns(), &DSB_FM_NAMESPACE::WindowsService::windowCreated, this,
            &BookMark::onWindowCreated, Qt::DirectConnection);

    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kRenameFile,
                                            BookMarkEventReceiver::instance(),
                                            &BookMarkEventReceiver::handleRenameFile);
    dpfInstance.eventDispatcher().subscribe(DSB_FM_NAMESPACE::BookMark::EventType::kBookMarkDisabled,
                                            BookMarkEventReceiver::instance(),
                                            &BookMarkEventReceiver::handleAddSchemeOfBookMarkDisabled);
}

bool BookMark::start()
{
    DSC_NAMESPACE::ActionInfo actionAddBookMark;
    actionAddBookMark.type = DFMBASE_NAMESPACE::ExtensionType::kSoAction;
    actionAddBookMark.createCb = BookMarkManager::bookMarkActionCreatedCallBack;
    actionAddBookMark.clickedCb = BookMarkManager::bookMarkActionClickedCallBack;
    BookMarkHelper::menuServIns()->regAction(actionAddBookMark);

    return true;
}

dpf::Plugin::ShutdownFlag BookMark::stop()
{
    return kSync;
}

void BookMark::onWindowCreated(quint64 winId)
{
    auto window = BookMarkHelper::winServIns()->findWindowById(winId);
    Q_ASSERT_X(window, "Computer", "Cannot find window by id");

    connect(window, &FileManagerWindow::sideBarInstallFinished, this,
            []() { BookMarkManager::instance()->addBookMarkItemsFromConfig(); },
            Qt::DirectConnection);
}
