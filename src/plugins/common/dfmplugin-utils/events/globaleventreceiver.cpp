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
#include "globaleventreceiver.h"
#include "utils/appendcompresshelper.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QUrl>
#include <QDir>
#include <QProcess>
#include <QDropEvent>
#include <QMimeData>

DPUTILS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

Q_DECLARE_METATYPE(const QMimeData *)
GlobalEventReceiver::GlobalEventReceiver(QObject *parent)
    : QObject(parent)
{
}

GlobalEventReceiver *GlobalEventReceiver::instance()
{
    static GlobalEventReceiver receiver;
    return &receiver;
}

void GlobalEventReceiver::initEventConnect()
{
    dpfSignalDispatcher->subscribe(GlobalEventType::kOpenAsAdmin,
                                   GlobalEventReceiver::instance(), &GlobalEventReceiver::handleOpenAsAdmin);

    // workspace
    dpfHookSequence->follow("dfmplugin_workspace", "hook_FileDragMove",
                            GlobalEventReceiver::instance(), &GlobalEventReceiver::handleSetMouseStyle);
    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_FileDrop",
                                   GlobalEventReceiver::instance(), &GlobalEventReceiver::handleDragDropCompress);

    // desktop
    dpfHookSequence->follow("ddplugin_canvas", "hook_CanvasView_FileDragMove",
                            GlobalEventReceiver::instance(), &GlobalEventReceiver::handleSetMouseStyleOnDesktop);
    dpfHookSequence->follow("ddplugin_canvas", "hook_CanvasView_FileDrop",
                            GlobalEventReceiver::instance(), &GlobalEventReceiver::handleDragDropCompressOnDesktop);
}

void GlobalEventReceiver::handleOpenAsAdmin(const QUrl &url)
{
    if (url.isEmpty() || !url.isValid()) {
        qWarning() << "Invalid Url: " << url;
        return;
    }

    QString localPath { url.toLocalFile() };
    if (!QDir(localPath).exists()) {
        qWarning() << "Url path not exists: " << localPath;
        return;
    }

    QProcess::startDetached("dde-file-manager-pkexec", { localPath });
}

bool GlobalEventReceiver::handleSetMouseStyle(const QUrl &toUrl, const QList<QUrl> &fromUrls, void *type)
{
    Qt::DropAction *dropAction = (Qt::DropAction *)type;
    return AppendCompressHelper::setMouseStyle(toUrl, fromUrls, *dropAction);
}

void GlobalEventReceiver::handleDragDropCompress(const QUrl &toUrl, const QList<QUrl> &fromUrls)
{
    AppendCompressHelper::dragDropCompress(toUrl, fromUrls);
}

bool GlobalEventReceiver::handleSetMouseStyleOnDesktop(int viewIndex, const QMimeData *mime, const QPoint &viewPos, void *extData)
{
    QVariantHash *data = (QVariantHash *)extData;
    if (data) {
        QUrl toUrl = data->value("hoverUrl").toUrl();
        QList<QUrl> fromUrls = mime->urls();
        Qt::DropAction *dropAction = (Qt::DropAction *)data->value("dropAction").toLongLong();
        if (dropAction) {
            return AppendCompressHelper::setMouseStyle(toUrl, fromUrls, *dropAction);
        }
    }
    return false;
}

bool GlobalEventReceiver::handleDragDropCompressOnDesktop(int viewIndex, const QMimeData *md, const QPoint &viewPos, void *extData)
{
    QVariantHash *data = (QVariantHash *)extData;
    if (data) {
        QUrl toUrl = data->value("dropUrl").toUrl();
        QList<QUrl> fromUrls = md->urls();
        AppendCompressHelper::dragDropCompress(toUrl, fromUrls);
    }

    return false;
}
