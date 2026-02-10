// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/ShareControlAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ShareControlAdaptor
 */

ShareControlAdaptor::ShareControlAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ShareControlAdaptor::~ShareControlAdaptor()
{
    // destructor
}

bool ShareControlAdaptor::CloseSmbShareByShareName(const QString &name, bool show)
{
    // handle method call org.deepin.Filemanager.UserShareManager.CloseSmbShareByShareName
    bool out0;
    QMetaObject::invokeMethod(parent(), "CloseSmbShareByShareName", Q_RETURN_ARG(bool, out0), Q_ARG(QString, name), Q_ARG(bool, show));
    return out0;
}

bool ShareControlAdaptor::EnableSmbServices()
{
    // handle method call org.deepin.Filemanager.UserShareManager.EnableSmbServices
    bool out0;
    QMetaObject::invokeMethod(parent(), "EnableSmbServices", Q_RETURN_ARG(bool, out0));
    return out0;
}

bool ShareControlAdaptor::IsUserSharePasswordSet(const QString &username)
{
    // handle method call org.deepin.Filemanager.UserShareManager.IsUserSharePasswordSet
    bool out0;
    QMetaObject::invokeMethod(parent(), "IsUserSharePasswordSet", Q_RETURN_ARG(bool, out0), Q_ARG(QString, username));
    return out0;
}

bool ShareControlAdaptor::SetUserSharePassword(const QString &name, const QString &passwd)
{
    // handle method call org.deepin.Filemanager.UserShareManager.SetUserSharePassword
    bool out0;
    QMetaObject::invokeMethod(parent(), "SetUserSharePassword", Q_RETURN_ARG(bool, out0), Q_ARG(QString, name), Q_ARG(QString, passwd));
    return out0;
}

