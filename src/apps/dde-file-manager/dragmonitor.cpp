// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dragmonitor.h"

#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDBusConnection>
#include <QDebug>

using namespace dfm_drag;

DragMoniter::DragMoniter(QObject *parent)
    : QObject { parent }, QDBusContext()
{
}

void DragMoniter::registerDBus()
{
    QDBusConnection con = QDBusConnection::sessionBus();
    if (!con.isConnected()) {
        qWarning() << "Cannot connect to the D-Bus session bus";
        return;
    }

    if (!con.registerService("org.deepin.filemanager.drag")) {
        qWarning() << "Cannot register D-Bus service:" << con.lastError().message();
        return;
    }

    if (!con.registerObject("/org/deepin/filemanager/drag",
                            this,
                            QDBusConnection::ExportScriptableSignals)) {
        qWarning() << "Cannot register D-Bus object:" << con.lastError().message();
        return;
    }

    qApp->installEventFilter(this);
}

void DragMoniter::unRegisterDBus()
{
    qApp->removeEventFilter(this);
    QDBusConnection con = QDBusConnection::sessionBus();
    con.unregisterObject("/org/deepin/filemanager/drag");
    con.unregisterService("org.deepin.filemanager.drag");
}

bool DragMoniter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent *e = dynamic_cast<QDragEnterEvent *>(event);
        if (e && e->mimeData() && e->mimeData()->hasUrls()) {
            QStringList str;
            const auto urls = e->mimeData()->urls();

            str.reserve(urls.size());

            for (const QUrl &u : urls) {
                if (u.isValid()) {
                    str << u.toString();
                }
            }

            if (!str.isEmpty())
                QMetaObject::invokeMethod(this, "DragEnter", Qt::QueuedConnection, Q_ARG(QStringList, str));

        }
    }

    return QObject::eventFilter(watched, event);
}
