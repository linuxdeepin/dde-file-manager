/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "secretmanager.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "interfaces/dfmstandardpaths.h"

#include "singleton.h"
#include "shutil/fileutils.h"

#include <QDebug>
#include <QJsonDocument>
#include <QFile>

SecretManager::SecretManager(QObject *parent) : QObject(parent)
{
    qDebug() << "Create SecretManager";
    initData();
    initConnect();
}

SecretManager::~SecretManager()
{

}

void SecretManager::initData()
{
    loadCache();
}

void SecretManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requsetCacheLoginData, this, &SecretManager::cacheSambaLoginData);
}

const SecretSchema *SecretManager::SMBSecretSchema()
{
    static const SecretSchema the_schema = {
        "org.gnome.keyring.NetworkPassword", SECRET_SCHEMA_DONT_MATCH_NAME,
        {
            {"user", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"domain", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"server", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"protocol", SECRET_SCHEMA_ATTRIBUTE_STRING }
        },
        0, 0, 0, 0, 0, 0, 0, 0
    };

    return &the_schema;
}

const SecretSchema *SecretManager::FTPSecretSchema()
{
    static const SecretSchema the_schema = {
        "org.gnome.keyring.NetworkPassword", SECRET_SCHEMA_DONT_MATCH_NAME,
        {
            {"user", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"server", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"protocol", SECRET_SCHEMA_ATTRIBUTE_STRING }
        },
        0, 0, 0, 0, 0, 0, 0, 0
    };

    return &the_schema;
}

const SecretSchema *SecretManager::VaultSecretSchema()
{
    static const SecretSchema the_schema = {
        "com.deepin.filemanager.VaultPassword", SECRET_SCHEMA_DONT_MATCH_NAME, {
            {"user", SECRET_SCHEMA_ATTRIBUTE_STRING }
        },
        0, 0, 0, 0, 0, 0, 0, 0
    };

    return &the_schema;
}

void SecretManager::on_password_cleared(GObject *source, GAsyncResult *result, gpointer unused)
{
    Q_UNUSED(source)
    Q_UNUSED(unused)

    qDebug() << "on_password_cleared";
    GError *error = nullptr;

    gboolean removed = secret_password_clear_finish(result, &error);

    qDebug() << removed;

    if (error != nullptr) {
        /* ... handle the failure here */
        g_error_free(error);

    } else {
        /* removed will be TRUE if a password was removed */
        qDebug() << "password was removed";
    }
}

bool SecretManager::storeVaultPassword(const DSecureString &string)
{
    GError *error = nullptr;
    secret_password_store_sync(VaultSecretSchema(), SECRET_COLLECTION_SESSION, "Vault session password", string.toStdString().c_str(),
                               nullptr, &error,
                               "user", "dde-file-manager",
                               nullptr);
    if (error != nullptr) {
        /* ... handle the failure here */
        g_error_free(error);
        return false;
    }

    return true;
}

DSecureString SecretManager::lookupVaultPassword()
{
    GError *error = nullptr;
    gchar *password = secret_password_lookup_sync(VaultSecretSchema(), {}, &error,
                                                  "user", "dde-file-manager",
                                                  NULL);
    DSecureString result(password);

    if (error != nullptr) {
        /* ... handle the failure here */
        g_error_free(error);
    } else if (password == nullptr) {
        /* password will be null, if no matching password found */
    } else {
        /* ... do something with the password */
        secret_password_free(password);
    }

    return result;
}

bool SecretManager::clearVaultPassword()
{
    GError *error = nullptr;

    /*
     * The variable argument list is the attributes used to later
     * lookup the password. These attributes must conform to the schema.
     */
    gboolean removed = secret_password_clear_sync(VaultSecretSchema(), nullptr, &error,
                                                  "user", "dde-file-manager",
                                                  nullptr);

    if (error != nullptr) {
        /* ... handle the failure here */
        g_error_free(error);
    } else {
        /* removed will be TRUE if a password was removed */
    }
    return removed;
}

void SecretManager::clearPasswordByLoginObj(const QJsonObject &obj)
{
    if (obj.value("protocol") == "smb") {
        secret_password_clear(SMBSecretSchema(), nullptr, on_password_cleared, nullptr,
                              "user", obj.value("user").toString().toStdString().c_str(),
                              "domain", obj.value("domain").toString().toStdString().c_str(),
                              "server", obj.value("server").toString().toStdString().c_str(),
                              "protocol", obj.value("protocol").toString().toStdString().c_str(),
                              nullptr);
    } else if (obj.value("protocol") == "ftp" || obj.value("protocol") == "sftp") {
        secret_password_clear(FTPSecretSchema(), nullptr, on_password_cleared, nullptr,
                              "user", obj.value("user").toString().toStdString().c_str(),
                              "server", obj.value("server").toString().toStdString().c_str(),
                              "protocol", obj.value("protocol").toString().toStdString().c_str(),
                              nullptr);
    }
}

QJsonObject SecretManager::getLoginData(const QString &id)
{
    return m_smbLoginObjs.value(id).toObject();
}

QJsonObject SecretManager::getLoginDatas()
{
    return m_smbLoginObjs;
}

QString SecretManager::cachePath()
{
    return QString("%1/samba.json").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath));
}

void SecretManager::cacheSambaLoginData(const QJsonObject &obj)
{
    QJsonValue v(obj);
    QString path = obj.value("id").toString();
    if (!path.isEmpty() && (path.startsWith("smb://") || path.startsWith("smb-share://"))){
        bool bend = path.endsWith("/");
        QString tmppath = bend ? path.left(path.length() - 1) : path;
        QString tmppathprev = tmppath.left(tmppath.lastIndexOf("/")+1);
        QString tmppathname = tmppath.mid(tmppath.lastIndexOf("/")+1);
        path = bend ? tmppathprev + tmppathname.toLower() + "/" : tmppathprev + tmppathname.toLower();
    }
    m_smbLoginObjs.insert(path, v);
    saveCache();
}

void SecretManager::loadCache()
{
    //Migration for old config files, and rmove that codes for further
    FileUtils::migrateConfigFileFromCache("samba");

    QFile file(cachePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't write samba file!";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    m_smbLoginObjs = jsonDoc.object();
    file.close();
    qDebug() << m_smbLoginObjs;
}

void SecretManager::saveCache()
{
    QFile file(cachePath());
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't write samba file!";
        return;
    }
    QJsonDocument jsonDoc(m_smbLoginObjs);
    file.write(jsonDoc.toJson());
    file.close();
}

