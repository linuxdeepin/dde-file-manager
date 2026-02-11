// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/config/dbus/TextIndexAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class TextIndexAdaptor
 */

TextIndexAdaptor::TextIndexAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

TextIndexAdaptor::~TextIndexAdaptor()
{
    // destructor
}

bool TextIndexAdaptor::CreateIndexTask(const QString &path)
{
    // handle method call org.deepin.Filemanager.TextIndex.CreateIndexTask
    bool out0;
    QMetaObject::invokeMethod(parent(), "CreateIndexTask", Q_RETURN_ARG(bool, out0), Q_ARG(QString, path));
    return out0;
}

QString TextIndexAdaptor::GetLastUpdateTime()
{
    // handle method call org.deepin.Filemanager.TextIndex.GetLastUpdateTime
    QString out0;
    QMetaObject::invokeMethod(parent(), "GetLastUpdateTime", Q_RETURN_ARG(QString, out0));
    return out0;
}

bool TextIndexAdaptor::HasRunningTask()
{
    // handle method call org.deepin.Filemanager.TextIndex.HasRunningTask
    bool out0;
    QMetaObject::invokeMethod(parent(), "HasRunningTask", Q_RETURN_ARG(bool, out0));
    return out0;
}

bool TextIndexAdaptor::IndexDatabaseExists()
{
    // handle method call org.deepin.Filemanager.TextIndex.IndexDatabaseExists
    bool out0;
    QMetaObject::invokeMethod(parent(), "IndexDatabaseExists", Q_RETURN_ARG(bool, out0));
    return out0;
}

bool TextIndexAdaptor::RemoveIndexTask(const QStringList &paths)
{
    // handle method call org.deepin.Filemanager.TextIndex.RemoveIndexTask
    bool out0;
    QMetaObject::invokeMethod(parent(), "RemoveIndexTask", Q_RETURN_ARG(bool, out0), Q_ARG(QStringList, paths));
    return out0;
}

bool TextIndexAdaptor::StopCurrentTask()
{
    // handle method call org.deepin.Filemanager.TextIndex.StopCurrentTask
    bool out0;
    QMetaObject::invokeMethod(parent(), "StopCurrentTask", Q_RETURN_ARG(bool, out0));
    return out0;
}

bool TextIndexAdaptor::UpdateIndexTask(const QString &path)
{
    // handle method call org.deepin.Filemanager.TextIndex.UpdateIndexTask
    bool out0;
    QMetaObject::invokeMethod(parent(), "UpdateIndexTask", Q_RETURN_ARG(bool, out0), Q_ARG(QString, path));
    return out0;
}

