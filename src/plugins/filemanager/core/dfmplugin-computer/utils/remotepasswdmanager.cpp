// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

QJsonObject RemotePasswdManager::getLoginInfo(const QString &uri)
{
    QMutexLocker locker(&mutex);
    for (auto key : smbObjs.keys()) {
        if (key.startsWith(uri) || uri.startsWith(key)) {
            auto obj = smbObjs.value(key).toObject();
            obj.insert("key", key);
            return obj;
        }
    }
    return smbObjs.value(uri).toObject();
}

QString RemotePasswdManager::parseServer(const QString &uri)
{
    // AppController::actionForgetPassword
    QStringList frags = uri.split("/");
    if (frags.count() < 3)
        return "";

    // just trans from old version. there is no example and no annotation...
    QString authField = frags.at(2);
    if (authField.contains(";")) {
        QStringList authFrags = authField.split(";");
        if (authFrags.count() >= 2) {
            QString userAuth = authFrags.at(1);
            if (userAuth.contains("@")) {
                QStringList userAuthFrags = userAuth.split("@");
                if (userAuthFrags.count() >= 2)
                    return userAuthFrags.at(1);
            }
        }
    } else {
        if (authField.contains("@")) {
            QStringList authFrags = authField.split("@");
            if (authFrags.count() >= 2)
                return authFrags.at(1);
        } else {
            return authField;
        }
    }

    return "";
}

QString RemotePasswdManager::configPath()
{
    static QString path = QString("%1/deepin/dde-file-manager/samba.json").arg(DFMBASE_NAMESPACE::StandardPaths::location(DFMBASE_NAMESPACE::StandardPaths::kApplicationConfigPath));
    return path;
}

void RemotePasswdManager::clearPasswd(const QString &uri)
{
    //    auto obj = getLoginInfo(uri);

    //    if (obj.isEmpty())
    //        return;

    //    QString user = obj.value("username").toString();
    //    QString domain = obj.value("domain").toString();
    //    QString key = obj.value("key").toString();
    QUrl url(uri);
    QString server = url.host();
    QString protocol = url.scheme();

    if (protocol == DFMBASE_NAMESPACE::Global::Scheme::kSmb) {
        secret_password_clear(smbSchema(), nullptr, onPasswdCleared, nullptr,
                              //                              "user", user.toStdString().c_str(),
                              //                              "domain", domain.toStdString().c_str(),
                              "server", server.toStdString().c_str(),
                              "protocol", protocol.toStdString().c_str(),
                              nullptr);
    } else if (protocol.endsWith(DFMBASE_NAMESPACE::Global::Scheme::kFtp)) {   // ftp && sftp
        secret_password_clear(ftpSchema(), nullptr, onPasswdCleared, nullptr,
                              //                              "user", user.toStdString().c_str(),
                              "server", server.toStdString().c_str(),
                              "protocol", protocol.toStdString().c_str(),
                              nullptr);
    }

    //    QMutexLocker locker(&mutex);
    //    smbObjs.remove(uri);
    //    saveConfig();
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
    qDebug() << "on password cleared: " << ret;
    if (err)
        qInfo() << "error while clear saved password: " << err->message;
}
