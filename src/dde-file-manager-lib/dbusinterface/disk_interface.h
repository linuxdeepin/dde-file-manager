/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c DiskInterface -p ./dbusinterface/disk_interface disk.xml
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef DISK_INTERFACE_H
#define DISK_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface local.DiskManager
 */
class DiskInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "local.DiskManager"; }

public:
    DiskInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~DiskInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> changeDiskPassword(const QString &oldPwd, const QString &newPwd)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(oldPwd) << QVariant::fromValue(newPwd);
        return asyncCallWithArgumentList(QStringLiteral("changeDiskPassword"), argumentList);
    }

    inline QDBusPendingReply<bool> checkAuthentication()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("checkAuthentication"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void finished(int code);
    void passwordChecked(bool result);
};

namespace local {
  typedef ::DiskInterface DiskManager;
}
#endif
