// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stringdecryptdbus.h"
#include "stringdecrypt_adapter.h"
#include "opensslhandler.h"

#include <QDBusConnection>

StringDecryptDBus::StringDecryptDBus(QObject *parent)
    : QObject(parent), QDBusContext()
{
    QDBusConnection::systemBus()
            .registerObject("/com/deepin/filemanager/daemon/EncryptKeyHelper",
                            this);
    new StringDecryptAdapter(this);
}

QString StringDecryptDBus::PublicKey()
{
    return daemonplugin_stringdecrypt::OpenSSLHandler::instance()
            ->pubKey()
            .toLocal8Bit()
            .toBase64();
}
