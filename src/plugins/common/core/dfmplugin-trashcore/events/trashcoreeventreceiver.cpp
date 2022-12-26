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
#include "dfm-base/file/local/localfilehandler.h"

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
    dpfSignalDispatcher->publish(GlobalEventType::kCleanTrash, windowId, QList<QUrl>(), AbstractJobHandler::DeleteDialogNoticeType::kEmptyTrash, nullptr);
}

bool TrashCoreEventReceiver::cutFileFromTrash(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    Q_UNUSED(windowId)
    Q_UNUSED(flags)

    if (sources.isEmpty())
        return true;

    const QUrl &urlSource = sources.first();
    if (urlSource.scheme() != DFMBASE_NAMESPACE::Global::Scheme::kTrash)
        return false;

    return dpfSignalDispatcher->publish(GlobalEventType::kRestoreFromTrash, windowId,
                                        sources, target, flags, nullptr);
}

bool TrashCoreEventReceiver::copyFromFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;

    const QUrl &fromUrl = sources.first();
    if (fromUrl.scheme() != TrashCoreHelper::scheme())
        return false;

    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kCopyFromTrash, windowId,
                                 sources, target, flags, nullptr);
    return true;
}
