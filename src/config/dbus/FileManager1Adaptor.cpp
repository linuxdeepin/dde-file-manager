// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/FileManager1Adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class FileManager1Adaptor
 */

FileManager1Adaptor::FileManager1Adaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

FileManager1Adaptor::~FileManager1Adaptor()
{
    // destructor
}

void FileManager1Adaptor::Open(const QStringList &Args)
{
    // handle method call org.freedesktop.FileManager1.Open
    QMetaObject::invokeMethod(parent(), "Open", Q_ARG(QStringList, Args));
}

void FileManager1Adaptor::ShowFolders(const QStringList &URIs, const QString &StartupId)
{
    // handle method call org.freedesktop.FileManager1.ShowFolders
    QMetaObject::invokeMethod(parent(), "ShowFolders", Q_ARG(QStringList, URIs), Q_ARG(QString, StartupId));
}

void FileManager1Adaptor::ShowItemProperties(const QStringList &URIs, const QString &StartupId)
{
    // handle method call org.freedesktop.FileManager1.ShowItemProperties
    QMetaObject::invokeMethod(parent(), "ShowItemProperties", Q_ARG(QStringList, URIs), Q_ARG(QString, StartupId));
}

void FileManager1Adaptor::ShowItems(const QStringList &URIs, const QString &StartupId)
{
    // handle method call org.freedesktop.FileManager1.ShowItems
    QMetaObject::invokeMethod(parent(), "ShowItems", Q_ARG(QStringList, URIs), Q_ARG(QString, StartupId));
}

void FileManager1Adaptor::Trash(const QStringList &URIs)
{
    // handle method call org.freedesktop.FileManager1.Trash
    QMetaObject::invokeMethod(parent(), "Trash", Q_ARG(QStringList, URIs));
}

