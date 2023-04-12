// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagmanagerdbus.h"
#include "serverplugin_tagdaemon_global.h"

TagManagerDBus::TagManagerDBus(QObject *parent)
    : QObject(parent)
{
}

QDBusVariant TagManagerDBus::Query(const quint32 &type, const QStringList value)
{

    QDBusVariant dbusVar {};
    return dbusVar;
}

bool TagManagerDBus::Insert(const quint32 &type, const QVariantMap value)
{

    return false;
}

bool TagManagerDBus::Delete(const quint32 &type, const QVariantMap value)
{

    return false;
}

bool TagManagerDBus::Update(const quint32 &type, const QVariantMap value)
{

    return false;
}

bool TagManagerDBus::CanTagFile(const QString &filePath)
{
    return {};
}
