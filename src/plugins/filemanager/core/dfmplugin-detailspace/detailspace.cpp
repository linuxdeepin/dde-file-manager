// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspace.h"
#include "utils/detailspacehelper.h"
#include "events/detailspaceeventreceiver.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

using namespace dfmplugin_detailspace;
DFMBASE_USE_NAMESPACE

void DetailSpace::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed, this, &DetailSpace::onWindowClosed, Qt::DirectConnection);
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
