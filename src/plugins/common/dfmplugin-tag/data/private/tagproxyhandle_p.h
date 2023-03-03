// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGPROXYHANDLE_P_H
#define TAGPROXYHANDLE_P_H

#include "dfmplugin_tag_global.h"

#include <QDBusVariant>

class TagDBusInterface;
class QDBusServiceWatcher;
namespace dfmplugin_tag {

class TagProxyHandle;
class TagProxyHandlePrivate : public QObject
{
    Q_OBJECT

public:
    enum ConnectType {
        kNoneConnection = -1,
        kAPIConnecting,
        kDBusConnecting
    };

    explicit TagProxyHandlePrivate(TagProxyHandle *qq, QObject *parent = nullptr);
    ~TagProxyHandlePrivate();

    bool isDBusRuning();
    void initConnection();

    void connectToDBus();
    void connectToAPI();
    void disconnCurrentConnections();

    QVariant parseDBusVariant(const QDBusVariant &var);

public:
    TagProxyHandle *q { nullptr };
    QScopedPointer<TagDBusInterface> tagDBusInterface;
    QScopedPointer<QDBusServiceWatcher> dbusWatcher;
    QList<QMetaObject::Connection> connections;
    int currentConnectionType = kNoneConnection;
};
}

#endif   // TAGPROXYHANDLE_P_H
