// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/RecentManagerAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class RecentManagerAdaptor
 */

RecentManagerAdaptor::RecentManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

RecentManagerAdaptor::~RecentManagerAdaptor()
{
    // destructor
}

void RecentManagerAdaptor::AddItem(const QVariantMap &item)
{
    // handle method call org.deepin.Filemanager.Daemon.RecentManager.AddItem
    QMetaObject::invokeMethod(parent(), "AddItem", Q_ARG(QVariantMap, item));
}

QVariantMap RecentManagerAdaptor::GetItemInfo(const QString &path)
{
    // handle method call org.deepin.Filemanager.Daemon.RecentManager.GetItemInfo
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "GetItemInfo", Q_RETURN_ARG(QVariantMap, out0), Q_ARG(QString, path));
    return out0;
}

QVariantList RecentManagerAdaptor::GetItemsInfo()
{
    // handle method call org.deepin.Filemanager.Daemon.RecentManager.GetItemsInfo
    QVariantList out0;
    QMetaObject::invokeMethod(parent(), "GetItemsInfo", Q_RETURN_ARG(QVariantList, out0));
    return out0;
}

QStringList RecentManagerAdaptor::GetItemsPath()
{
    // handle method call org.deepin.Filemanager.Daemon.RecentManager.GetItemsPath
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "GetItemsPath", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

void RecentManagerAdaptor::PurgeItems()
{
    // handle method call org.deepin.Filemanager.Daemon.RecentManager.PurgeItems
    QMetaObject::invokeMethod(parent(), "PurgeItems");
}

qlonglong RecentManagerAdaptor::Reload()
{
    // handle method call org.deepin.Filemanager.Daemon.RecentManager.Reload
    qlonglong out0;
    QMetaObject::invokeMethod(parent(), "Reload", Q_RETURN_ARG(qlonglong, out0));
    return out0;
}

void RecentManagerAdaptor::RemoveItems(const QStringList &hrefs)
{
    // handle method call org.deepin.Filemanager.Daemon.RecentManager.RemoveItems
    QMetaObject::invokeMethod(parent(), "RemoveItems", Q_ARG(QStringList, hrefs));
}

