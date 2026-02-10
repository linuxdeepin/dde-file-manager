// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/TagManagerAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class TagManagerAdaptor
 */

TagManagerAdaptor::TagManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

TagManagerAdaptor::~TagManagerAdaptor()
{
    // destructor
}

bool TagManagerAdaptor::Delete(int opt, const QVariantMap &value)
{
    // handle method call org.deepin.Filemanager.Daemon.TagManager.Delete
    bool out0;
    QMetaObject::invokeMethod(parent(), "Delete", Q_RETURN_ARG(bool, out0), Q_ARG(int, opt), Q_ARG(QVariantMap, value));
    return out0;
}

bool TagManagerAdaptor::Insert(int opt, const QVariantMap &value)
{
    // handle method call org.deepin.Filemanager.Daemon.TagManager.Insert
    bool out0;
    QMetaObject::invokeMethod(parent(), "Insert", Q_RETURN_ARG(bool, out0), Q_ARG(int, opt), Q_ARG(QVariantMap, value));
    return out0;
}

QDBusVariant TagManagerAdaptor::Query(int opt)
{
    // handle method call org.deepin.Filemanager.Daemon.TagManager.Query
    QDBusVariant out0;
    QMetaObject::invokeMethod(parent(), "Query", Q_RETURN_ARG(QDBusVariant, out0), Q_ARG(int, opt));
    return out0;
}

QDBusVariant TagManagerAdaptor::Query(int opt, const QStringList &value)
{
    // handle method call org.deepin.Filemanager.Daemon.TagManager.Query
    QDBusVariant out0;
    QMetaObject::invokeMethod(parent(), "Query", Q_RETURN_ARG(QDBusVariant, out0), Q_ARG(int, opt), Q_ARG(QStringList, value));
    return out0;
}

bool TagManagerAdaptor::Update(int opt, const QVariantMap &value)
{
    // handle method call org.deepin.Filemanager.Daemon.TagManager.Update
    bool out0;
    QMetaObject::invokeMethod(parent(), "Update", Q_RETURN_ARG(bool, out0), Q_ARG(int, opt), Q_ARG(QVariantMap, value));
    return out0;
}

