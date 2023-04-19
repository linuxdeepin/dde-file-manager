// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGPROXYHANDLE_P_H
#define TAGPROXYHANDLE_P_H

#include "dfmplugin_tag_global.h"
#include "tagmanager_interface.h"

#include <QDBusVariant>

using TagManagerDBusInterface = OrgDeepinFilemanagerServerTagManagerInterface;

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
