/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "remotepasswdmanager.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/dfm_global_defines.h"

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
    //    loadConfig();
}

void RemotePasswdManager::loadConfig()
{
    // SecretManager::loadCache

    QFile file(configPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qInfo() << "cannot open samba.json in readonly mode";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc(QJsonDocument::fromJson(data));
    smbObjs = doc.object();
    qDebug() << "read samba config: \n"
             << smbObjs;
}

void RemotePasswdManager::saveConfig()
{
    // SecretManager::saveCache

    QFile file(configPath());
    if (!file.open(QIODevice::WriteOnly)) {
        qInfo() << "cannot open samba.json in writeonly mode";
        return;
    }

    QJsonDocument doc(smbObjs);
    file.write(doc.toJson());
    file.close();
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
    QString server = parseServer(uri);
    QString protocol = uri.mid(0, uri.indexOf("://"));

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
