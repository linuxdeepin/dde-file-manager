/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfmsearchcrumbcontroller.h"

#include "dfmcrumbbar.h"
#include "dfmeventdispatcher.h"

#include "views/dfilemanagerwindow.h"
#include "views/dfileview.h"
#include "dfilesystemmodel.h"

#include <QDebug>

DFM_BEGIN_NAMESPACE

DFMSearchCrumbController::DFMSearchCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMSearchCrumbController::~DFMSearchCrumbController()
{

}

void DFMSearchCrumbController::processAction(DFMCrumbInterface::ActionType type)
{
    switch (type) {
    case EscKeyPressed:
    case ClearButtonPressed: {
        crumbBar()->hideAddressBar();
        DFileManagerWindow *window = qobject_cast<DFileManagerWindow*>(crumbBar()->window());

        if (!window) {
            break;
        }

        const DUrl &current_url = window->currentUrl();
        auto event = dMakeEventPointer<DFMChangeCurrentUrlEvent>(crumbBar(), current_url.searchTargetUrl(), window);
        DFMEventDispatcher::instance()->processEvent(event);
        break;
    }
    case PauseButtonClicked: {
        DFileManagerWindow *window = qobject_cast<DFileManagerWindow*>(crumbBar()->window());
        if (!window)
            break;

        DFileView *fileView = dynamic_cast<DFileView *>(window->getFileView());
        if (!fileView)
            break;

        fileView->model()->stopCurrentJob();
        break;
    }
    case AddressBarLostFocus:
        // Do nothing here.
        break;
    //全量枚举case，因此为解决警告删除default
    }
}

void DFMSearchCrumbController::crumbUrlChangedBehavior(const DUrl url)
{
    crumbBar()->showAddressBar(url.searchKeyword());
}

bool DFMSearchCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == SEARCH_SCHEME);
}

DFM_END_NAMESPACE
