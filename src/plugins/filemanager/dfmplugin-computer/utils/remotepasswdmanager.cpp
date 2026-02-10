// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remotepasswdmanager.h"

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dfm_global_defines.h>

#include <QFile>
#include <QDebug>
#include <QJsonDocument>

using namespace dfmplugin_computer;

RemotePasswdManager *RemotePasswdManager::instance()
{
    static RemotePasswdManager ins;
    return &ins;
}

RemotePasswdManager::RemotePasswdManager(QObject *parent)
    : QObject(parent)
{
}

void RemotePasswdManager::clearPasswd(const QString &uri)
{
    QUrl url(uri);
    QString server = url.host();
    QString protocol = url.scheme();

    if (protocol == DFMBASE_NAMESPACE::Global::Scheme::kSmb) {
        fmDebug() << "Clearing SMB password for server:" << server;
        secret_password_clear(smbSchema(), nullptr, onPasswdCleared, nullptr,
                              //                              "user", user.toStdString().c_str(),
                              //                              "domain", domain.toStdString().c_str(),
                              "server", server.toStdString().c_str(),
                              "protocol", protocol.toStdString().c_str(),
                              nullptr);
    } else if (protocol.endsWith(DFMBASE_NAMESPACE::Global::Scheme::kFtp)) {   // ftp && sftp
        fmDebug() << "Clearing FTP/SFTP password for server:" << server;
        secret_password_clear(ftpSchema(), nullptr, onPasswdCleared, nullptr,
                              //                              "user", user.toStdString().c_str(),
                              "server", server.toStdString().c_str(),
                              "protocol", protocol.toStdString().c_str(),
                              nullptr);
    } else {
        fmWarning() << "Unsupported protocol for password clearing:" << protocol;
    }
}

const SecretSchema *RemotePasswdManager::smbSchema()
{
    static const SecretSchema schema {
        "org.gnome.keyring.NetworkPassword",   // name
        SECRET_SCHEMA_DONT_MATCH_NAME,   // flag
        { { "user", SECRET_SCHEMA_ATTRIBUTE_STRING },   // attrs
          { "domain", SECRET_SCHEMA_ATTRIBUTE_STRING },
          { "server", SECRET_SCHEMA_ATTRIBUTE_STRING },
          { "protocol", SECRET_SCHEMA_ATTRIBUTE_STRING } },
        0,   // reserved
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
    return &schema;
}

const SecretSchema *RemotePasswdManager::ftpSchema()
{
    static const SecretSchema schema {
        "org.gnome.keyring.NetworkPassword",   // name
        SECRET_SCHEMA_DONT_MATCH_NAME,   // flag
        { { "user", SECRET_SCHEMA_ATTRIBUTE_STRING },   // attrs
          { "server", SECRET_SCHEMA_ATTRIBUTE_STRING },
          { "protocol", SECRET_SCHEMA_ATTRIBUTE_STRING } },
        0,   // reserved
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
    return &schema;
}

void RemotePasswdManager::onPasswdCleared(GObject *obj, GAsyncResult *res, gpointer data)
{
    Q_UNUSED(obj)
    Q_UNUSED(data)

    GError_autoptr err = nullptr;
    bool ret = secret_password_clear_finish(res, &err);
    fmInfo() << "on password cleared: " << ret;
    if (err)
        fmCritical() << "Error occurred while clearing saved password:" << err->message;
}
