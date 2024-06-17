// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef STRINGENCRYPTDBUS_H
#define STRINGENCRYPTDBUS_H

#include <QObject>
#include <QDBusContext>

class StringDecryptDBus : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.daemon.EncryptKeyHelper")

public:
    explicit StringDecryptDBus(QObject *parent = nullptr);

public Q_SLOTS:
    QString PublicKey();
};

#endif   // STRINGENCRYPTDBUS_H
