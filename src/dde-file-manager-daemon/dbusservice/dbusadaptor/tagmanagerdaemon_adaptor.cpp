// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusadaptor/tagmanagerdaemon_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class TagManagerDaemonAdaptor
 */

TagManagerDaemonAdaptor::TagManagerDaemonAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

TagManagerDaemonAdaptor::~TagManagerDaemonAdaptor()
{
    // destructor
}

QDBusVariant TagManagerDaemonAdaptor::disposeClientData(const QVariantMap &filesAndTags, qulonglong type)
{
    // handle method call com.deepin.filemanager.daemon.TagManagerDaemon.disposeClientData
    QDBusVariant result;
    QMetaObject::invokeMethod(parent(), "disposeClientData", Q_RETURN_ARG(QDBusVariant, result), Q_ARG(QVariantMap, filesAndTags), Q_ARG(qulonglong, type));
    return result;
}

