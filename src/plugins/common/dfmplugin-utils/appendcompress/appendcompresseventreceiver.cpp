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
#include "appendcompresseventreceiver.h"
#include "appendcompress/appendcompresshelper.h"

#include <dfm-framework/dpf.h>

#include <QDropEvent>
#include <QMimeData>

DPUTILS_USE_NAMESPACE

Q_DECLARE_METATYPE(const QMimeData *)
Q_DECLARE_METATYPE(Qt::DropAction *)

void AppendCompressEventReceiver::initEventConnect()
{
    // workspace
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FileDragMove",
                            this, &AppendCompressEventReceiver::handleSetMouseStyle);
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FileDrop",
                            this, &AppendCompressEventReceiver::handleDragDropCompress);

    // desktop
    dpfHookSequence->follow("ddplugin_canvas", "hook_CanvasView_DragMove",
                            this, &AppendCompressEventReceiver::handleSetMouseStyleOnDesktop);
    dpfHookSequence->follow("ddplugin_canvas", "hook_CanvasView_DropData",
                            this, &AppendCompressEventReceiver::handleDragDropCompressOnDesktop);
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

AppendCompressEventReceiver::AppendCompressEventReceiver(QObject *parent)
    : QObject(parent)
{
}
