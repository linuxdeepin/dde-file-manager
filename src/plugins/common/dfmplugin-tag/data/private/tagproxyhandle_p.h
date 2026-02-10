// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGPROXYHANDLE_P_H
#define TAGPROXYHANDLE_P_H

#include "dfmplugin_tag_global.h"

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include "tagmanager_interface.h"
#else
#    include "tagmanager_interface_qt6.h"
#endif

#include <QDBusVariant>

using TagManagerDBusInterface = OrgDeepinFilemanagerDaemonTagManagerInterface;

class QDBusServiceWatcher;
namespace dfmplugin_tag {

class TagProxyHandle;
class TagProxyHandlePrivate : public QObject
{
    Q_OBJECT

public:
    explicit TagProxyHandlePrivate(TagProxyHandle *qq, QObject *parent = nullptr);
    ~TagProxyHandlePrivate();

    bool isDBusRuning();
    void initConnection();

    void connectToDBus();
    void disconnCurrentConnections();

    QVariant parseDBusVariant(const QDBusVariant &var);

public:
    TagProxyHandle *q { nullptr };
    QScopedPointer<TagManagerDBusInterface> tagDBusInterface;
    QScopedPointer<QDBusServiceWatcher> dbusWatcher;
    QList<QMetaObject::Connection> connections;
};
}

#endif   // TAGPROXYHANDLE_P_H
