// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/AccessControlAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class AccessControlAdaptor
 */

AccessControlAdaptor::AccessControlAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

AccessControlAdaptor::~AccessControlAdaptor()
{
    // destructor
}

void AccessControlAdaptor::ChangeDiskPassword(const QString &oldPwd, const QString &newPwd)
{
    // handle method call org.deepin.Filemanager.AccessControlManager.ChangeDiskPassword
    QMetaObject::invokeMethod(parent(), "ChangeDiskPassword", Q_ARG(QString, oldPwd), Q_ARG(QString, newPwd));
}

bool AccessControlAdaptor::Chmod(const QString &path, uint mode)
{
    // handle method call org.deepin.Filemanager.AccessControlManager.Chmod
    bool out0;
    QMetaObject::invokeMethod(parent(), "Chmod", Q_RETURN_ARG(bool, out0), Q_ARG(QString, path), Q_ARG(uint, mode));
    return out0;
}

