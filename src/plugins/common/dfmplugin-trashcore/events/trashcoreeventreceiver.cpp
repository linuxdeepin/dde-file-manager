// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcoreeventreceiver.h"
#include "utils/trashcorehelper.h"
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/file/local/localfilehandler.h>

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
