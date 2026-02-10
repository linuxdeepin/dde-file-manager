// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appendcompresseventreceiver.h"
#include "appendcompress/appendcompresshelper.h"

#include <dfm-framework/dpf.h>

#include <QDropEvent>
#include <QMimeData>

using namespace dfmplugin_utils;

Q_DECLARE_METATYPE(const QMimeData *)
Q_DECLARE_METATYPE(Qt::DropAction *)

void AppendCompressEventReceiver::initEventConnect()
{
    // workspace
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDragMove",
                            this, &AppendCompressEventReceiver::handleSetMouseStyle);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_FileDrop",
                            this, &AppendCompressEventReceiver::handleDragDropCompress);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_DragDrop_IsDrop",
                            this, &AppendCompressEventReceiver::handleIsDrop);

    // desktop
    auto canvasEventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_canvas", "hook_CanvasView_DragMove") };
    if (canvasEventID != DPF_NAMESPACE::EventTypeScope::kInValid) {
        dpfHookSequence->follow("ddplugin_canvas", "hook_CanvasView_DragMove",
                                this, &AppendCompressEventReceiver::handleSetMouseStyleOnDesktop);
        dpfHookSequence->follow("ddplugin_canvas", "hook_CanvasView_DropData",
                                this, &AppendCompressEventReceiver::handleDragDropCompressOnDesktop);
    }

    // organizer
    auto organizerEventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_organizer", "hook_CollectionView_DragMove") };
    if (organizerEventID != DPF_NAMESPACE::EventTypeScope::kInValid) {
        dpfHookSequence->follow("ddplugin_organizer", "hook_CollectionView_DragMove",
                                this, &AppendCompressEventReceiver::handleSetMouseStyleOnOrganizer);
        dpfHookSequence->follow("ddplugin_organizer", "hook_CollectionView_DropData",
                                this, &AppendCompressEventReceiver::handleDragDropCompressOnDesktop);
    }
}

bool AppendCompressEventReceiver::handleSetMouseStyle(const QList<QUrl> &fromUrls, const QUrl &toUrl, Qt::DropAction *type)
{
    return AppendCompressHelper::setMouseStyle(toUrl, fromUrls, type);
}

bool AppendCompressEventReceiver::handleDragDropCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl)
{
    return AppendCompressHelper::dragDropCompress(toUrl, fromUrls);
}

bool AppendCompressEventReceiver::handleSetMouseStyleOnDesktop(int viewIndex, const QMimeData *mime, const QPoint &viewPos, void *extData)
{
    Q_UNUSED(viewIndex)
    Q_UNUSED(viewPos)

    QVariantHash *data = static_cast<QVariantHash *>(extData);
    if (data) {
        QUrl toUrl = data->value("hoverUrl").toUrl();
        QList<QUrl> fromUrls = mime->urls();
        Qt::DropAction *dropAction = reinterpret_cast<Qt::DropAction *>(data->value("dropAction").toLongLong());
        if (dropAction) {
            return AppendCompressHelper::setMouseStyle(toUrl, fromUrls, dropAction);
        }
    }
    return false;
}

bool AppendCompressEventReceiver::handleDragDropCompressOnDesktop(int viewIndex, const QMimeData *md, const QPoint &viewPos, void *extData)
{
    Q_UNUSED(viewIndex)
    Q_UNUSED(viewPos)

    QVariantHash *data = static_cast<QVariantHash *>(extData);
    if (data) {
        QUrl toUrl = data->value("dropUrl").toUrl();
        QList<QUrl> fromUrls = md->urls();
        return AppendCompressHelper::dragDropCompress(toUrl, fromUrls);
    }

    return false;
}

bool AppendCompressEventReceiver::handleSetMouseStyleOnOrganizer(const QString &viewId, const QMimeData *mime, const QPoint &viewPos, void *extData)
{
    QVariantHash *data = static_cast<QVariantHash *>(extData);
    if (data) {
        QUrl toUrl = data->value("hoverUrl").toUrl();
        QList<QUrl> fromUrls = mime->urls();
        Qt::DropAction *dropAction = reinterpret_cast<Qt::DropAction *>(data->value("dropAction").toLongLong());
        if (dropAction) {
            return AppendCompressHelper::setMouseStyle(toUrl, fromUrls, dropAction);
        }
    }
    return false;
}

bool AppendCompressEventReceiver::handleDragDropCompressOnOsrganizer(const QString &viewId, const QMimeData *md, const QPoint &viewPos, void *extData)
{
    QVariantHash *data = static_cast<QVariantHash *>(extData);
    if (data) {
        QUrl toUrl = data->value("dropUrl").toUrl();
        QList<QUrl> fromUrls = md->urls();
        return AppendCompressHelper::dragDropCompress(toUrl, fromUrls);
    }

    return false;
}

bool AppendCompressEventReceiver::handleIsDrop(const QUrl &url)
{
    return AppendCompressHelper::isCompressedFile(url);
}

AppendCompressEventReceiver::AppendCompressEventReceiver(QObject *parent)
    : QObject(parent)
{
}
