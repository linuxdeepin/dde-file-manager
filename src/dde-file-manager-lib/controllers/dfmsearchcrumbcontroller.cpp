// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
