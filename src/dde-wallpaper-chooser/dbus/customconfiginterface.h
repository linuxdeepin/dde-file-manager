// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMCONFIGINTERFACE_H
#define CUSTOMCONFIGINTERFACE_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

/*
 * Proxy class for interface com.deepin.CustomConfig
 */

class CustomConfigInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.CustomConfig"; }
public:
    CustomConfigInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);
    ~CustomConfigInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> StartCustomConfig(const QString &name)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(name);
        return asyncCallWithArgumentList(QStringLiteral("StartCustomConfig"), argumentList);
    }
};

namespace com {
  namespace deepin {
    typedef ::CustomConfigInterface CustomConfig;
  }
}

#endif // CUSTOMCONFIGINTERFACE_H
