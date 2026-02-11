// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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

void DragMoniter::start()
{
    qApp->installEventFilter(this);
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
                QMetaObject::invokeMethod(this, "dragEnter", Qt::QueuedConnection, Q_ARG(QStringList, str));
        }
    }

    return QObject::eventFilter(watched, event);
}
