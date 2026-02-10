// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/OperationsStackManagerAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class OperationsStackManagerAdaptor
 */

OperationsStackManagerAdaptor::OperationsStackManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

OperationsStackManagerAdaptor::~OperationsStackManagerAdaptor()
{
    // destructor
}

void OperationsStackManagerAdaptor::CleanOperations()
{
    // handle method call org.deepin.Filemanager.Daemon.OperationsStackManager.CleanOperations
    QMetaObject::invokeMethod(parent(), "CleanOperations");
}

void OperationsStackManagerAdaptor::CleanOperationsByUrl(const QStringList &urls)
{
    // handle method call org.deepin.Filemanager.Daemon.OperationsStackManager.CleanOperationsByUrl
    QMetaObject::invokeMethod(parent(), "CleanOperationsByUrl", Q_ARG(QStringList, urls));
}

QVariantMap OperationsStackManagerAdaptor::RevocationOperations()
{
    // handle method call org.deepin.Filemanager.Daemon.OperationsStackManager.RevocationOperations
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "RevocationOperations", Q_RETURN_ARG(QVariantMap, out0));
    return out0;
}

QVariantMap OperationsStackManagerAdaptor::RevocationRedoOperations()
{
    // handle method call org.deepin.Filemanager.Daemon.OperationsStackManager.RevocationRedoOperations
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "RevocationRedoOperations", Q_RETURN_ARG(QVariantMap, out0));
    return out0;
}

void OperationsStackManagerAdaptor::SaveOperations(const QVariantMap &values)
{
    // handle method call org.deepin.Filemanager.Daemon.OperationsStackManager.SaveOperations
    QMetaObject::invokeMethod(parent(), "SaveOperations", Q_ARG(QVariantMap, values));
}

void OperationsStackManagerAdaptor::SaveRedoOperations(const QVariantMap &values)
{
    // handle method call org.deepin.Filemanager.Daemon.OperationsStackManager.SaveRedoOperations
    QMetaObject::invokeMethod(parent(), "SaveRedoOperations", Q_ARG(QVariantMap, values));
}

