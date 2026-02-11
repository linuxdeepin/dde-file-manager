// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGER1DBUS_H
#define FILEMANAGER1DBUS_H

#include <QObject>
#include <QDBusMessage>

// NOTE:
// Detail see: https://www.freedesktop.org/wiki/Specifications/file-manager-interface/

class FileManager1DBus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")

public:
    explicit FileManager1DBus(QObject *parent = nullptr);

public slots:
    void ShowFolders(const QStringList &URIs, const QString &StartupId);
    void ShowItemProperties(const QStringList &URIs, const QString &StartupId);
    void ShowItems(const QStringList &URIs, const QString &StartupId);
    void Trash(const QStringList &URIs);
    void Open(const QStringList &URIs);
};

#endif   // FILEMANAGER1DBUS_H
