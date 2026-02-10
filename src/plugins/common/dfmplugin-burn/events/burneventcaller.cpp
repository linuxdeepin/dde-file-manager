// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burneventcaller.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-framework/dpf.h>

using namespace dfmplugin_burn;
DFMBASE_USE_NAMESPACE

void BurnEventCaller::sendPasteFiles(const QList<QUrl> &urls, const QUrl &dest, bool isCopy)
{
    if (isCopy)
        dpfSignalDispatcher->publish(GlobalEventType::kCopy, 0, urls, dest, AbstractJobHandler::JobFlag::kNoHint, nullptr);
    else
        dpfSignalDispatcher->publish(GlobalEventType::kCutFile, 0, urls, dest, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}
