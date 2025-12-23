// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspace.h"
#include "utils/detailspacehelper.h"
#include "events/detailspaceeventreceiver.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <dfm-framework/dpf.h>

using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

namespace dfmplugin_detailspace {
DFM_LOG_REGISTER_CATEGORY(DPDETAILSPACE_NAMESPACE)
DFMBASE_USE_NAMESPACE

void DetailSpace::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &DetailSpace::onWindowClosed, Qt::DirectConnection);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened, this, &DetailSpace::onWindowOpened, Qt::DirectConnection);
    DetailSpaceEventReceiver::instance().connectService();
}

bool DetailSpace::start()
{
    return true;
}

void DetailSpace::onWindowClosed(quint64 windId)
{
    DetailSpaceHelper::removeDetailSpace(windId);
}

void DetailSpace::onWindowOpened(quint64 windId)
{
    auto window = FMWindowsIns.findWindowById(windId);
    if (!window)
        return;

    // Connect to detailSpaceHideByDrag signal to handle drag-to-hide
    connect(window, &FileManagerWindow::detailSpaceHideByDrag, this, []() {
        // Sync titlebar button state via DConfig
        DConfigManager::instance()->setValue(kViewDConfName, kDisplayPreviewVisibleKey, false);
    });
}

}   // namespace dfmplugin_detailspace
