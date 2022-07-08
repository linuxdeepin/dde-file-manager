/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "trashcoreeventreceiver.h"
#include "utils/trashcorehelper.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QUrl>

#include <functional>

using namespace dfmplugin_trashcore;
DFMBASE_USE_NAMESPACE

TrashCoreEventReceiver::TrashCoreEventReceiver(QObject *parent)
    : QObject(parent)
{
}

TrashCoreEventReceiver *TrashCoreEventReceiver::instance()
{
    static TrashCoreEventReceiver receiver;
    return &receiver;
}

void TrashCoreEventReceiver::handleEmptyTrash(const quint64 windowId)
{
    QUrl url = TrashCoreHelper::toLocalFile(TrashCoreHelper::rootUrl());
    QList<QUrl> urls;
    urls.append(std::move(url));
    dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash, windowId, urls, AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);
}
