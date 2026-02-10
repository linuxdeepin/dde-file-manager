// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/MountControlAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class MountControlAdaptor
 */

MountControlAdaptor::MountControlAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

MountControlAdaptor::~MountControlAdaptor()
{
    // destructor
}

QVariantMap MountControlAdaptor::Mount(const QString &path, const QVariantMap &opts)
{
    // handle method call org.deepin.Filemanager.MountControl.Mount
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "Mount", Q_RETURN_ARG(QVariantMap, out0), Q_ARG(QString, path), Q_ARG(QVariantMap, opts));
    return out0;
}

QStringList MountControlAdaptor::SupportedFileSystems()
{
    // handle method call org.deepin.Filemanager.MountControl.SupportedFileSystems
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "SupportedFileSystems", Q_RETURN_ARG(QStringList, out0));
    return out0;
}

QVariantMap MountControlAdaptor::Unmount(const QString &path, const QVariantMap &opts)
{
    // handle method call org.deepin.Filemanager.MountControl.Unmount
    QVariantMap out0;
    QMetaObject::invokeMethod(parent(), "Unmount", Q_RETURN_ARG(QVariantMap, out0), Q_ARG(QString, path), Q_ARG(QVariantMap, opts));
    return out0;
}

