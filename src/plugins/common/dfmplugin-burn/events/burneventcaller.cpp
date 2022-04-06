/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "burneventcaller.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <dfm-framework/framework.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void BurnEventCaller::sendDeleteFiles(const QList<QUrl> &files)
{
    qInfo() << "Delete disc cache: " << files;
    dpfInstance.eventDispatcher().publish(GlobalEventType::kDeleteFiles, 0, files, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void BurnEventCaller::sendPasteFiles(const QList<QUrl> &urls, const QUrl &dest, bool isCopy)
{
    if (isCopy)
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCopy, 0, urls, dest, AbstractJobHandler::JobFlag::kNoHint, nullptr);
    else
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCutFile, 0, urls, dest, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}
