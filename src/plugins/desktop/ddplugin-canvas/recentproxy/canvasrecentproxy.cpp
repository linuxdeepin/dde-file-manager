// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasrecentproxy.h"
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusError>
#include <QVariant>
#include <QDBusPendingCall>
#include <QUrl>

namespace ddplugin_canvas {

void CanvasRecentProxy::handleReloadRecentFiles(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg);
    if (!ok || srcUrls.isEmpty())
        return;

    QDBusMessage message = QDBusMessage::createMethodCall("org.deepin.Filemanager.Daemon",
                                                          "/org/deepin/Filemanager/Daemon/RecentManager",
                                                          "org.deepin.Filemanager.Daemon.RecentManager",
                                                          "Reload");
    QDBusConnection::sessionBus().asyncCall(message);
}

CanvasRecentProxy::CanvasRecentProxy(QObject *parent)
    : QObject(parent)
{
}

CanvasRecentProxy::~CanvasRecentProxy()
{
}
} // namespace ddplugin_canvas
