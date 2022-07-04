/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "vaultdbusresponse.h"
#include "vaultconfig.h"
#include "operatorcenter.h"
#include "controllers/vaulterrorcode.h"

#include <QDBusConnection>
#include <QDBusArgument>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QStorageInfo>
#include <QJsonParseError>
#include <QJsonObject>
#include <QDateTime>

VaultDbusResponse::VaultDbusResponse(QObject *parent) : QObject(parent)
{

}

VaultDbusResponse *VaultDbusResponse::instance()
{
    static VaultDbusResponse ins;
    return &ins;
}

bool VaultDbusResponse::connectLockScreenDBus()
{
    return QDBusConnection::sessionBus().connect(
                "com.deepin.SessionManager",
                "/com/deepin/SessionManager",
                "org.freedesktop.DBus.Properties",
                "PropertiesChanged", "sa{sv}as",
                this,
                SLOT(responseLockScreenDBus(QDBusMessage)));
}

bool VaultDbusResponse::transparentUnlockVault()
{
    VaultState st = state();
    if (st != Encrypted) {
        qWarning() << "Vault: Unlock vault failed, current state is " << st;
        return false;
    }
    // 判断保险箱加密方式
    VaultConfig config;
    QString encryptionMethod = config.get(CONFIG_NODE_NAME, CONFIG_KEY_ENCRYPTION_METHOD, QVariant("NoExist")).toString();
    if (encryptionMethod == CONFIG_METHOD_VALUE_TRANSPARENT) {
        // 获取密码
        const QString &passwd = OperatorCenter::getInstance()->getPasswordFromKeyring();
        if (passwd.isEmpty()) {
            qWarning() << "Vault: Get password is empty, can not unlock vault!";
            return false;
        }

        const QString &basedir = VAULT_BASE_PATH + QDir::separator() + VAULT_ENCRYPY_DIR_NAME;
        const QString &mountdir = VAULT_BASE_PATH + QDir::separator() + VAULT_DECRYPT_DIR_NAME;
        // 创建挂载目录
        if (!QFile::exists(mountdir)) {
            QDir().mkpath(mountdir);
        }
        qInfo() << "临时打印密码，方便调试:" << passwd;
        // 解锁保险箱
        int result = unlockVault(basedir, mountdir, passwd);
        if (!result) {
            qInfo() << "Vault: Unlock vault success!";
            // 更新保险箱访问时间时间
            ChangeJson(VAULT_TIME_CONFIG_FILE_SUFFIX,
                       QString("VaultTime"),
                       QString("InterviewTime"),
                       QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            return true;
        } else {
            qWarning() << "Vault: Unlock vault failed, error code: " << result;
        }
    }

    return false;
}

void VaultDbusResponse::responseLockScreenDBus(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count())
        return;

    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != "com.deepin.SessionManager")
        return;

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach (const QString &prop, keys) {
        if (prop == "Locked") {
            bool isLocked = changedProps[prop].toBool();
            if (!isLocked) {
                transparentUnlockVault();
            }
        }
    }
}

int VaultDbusResponse::unlockVault(const QString &basedir, const QString &mountdir, const QString &passwd)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        qWarning() << "Vault: Lock vault failed, can not find the cryfs program!";
        return static_cast<int>(ErrorCode::CryfsNotExist);
    }

    QStringList arguments;
    arguments << basedir << mountdir;

    QProcess proc;
    proc.setEnvironment({"CRYFS_FRONTEND=noninteractive"});
    proc.start(cryfsBinary, arguments);
    proc.waitForStarted();
    proc.write(passwd.toUtf8());
    proc.waitForBytesWritten();
    proc.closeWriteChannel();
    proc.waitForFinished();
    proc.terminate();

    if(proc.exitStatus() == QProcess::NormalExit)
        return proc.exitCode();
    else
        return -1;
}

VaultDbusResponse::VaultState VaultDbusResponse::state()
{
    if (QStandardPaths::findExecutable("cryfs").isEmpty())
        return NotAvailable;

    const QString &configFilePath = VAULT_BASE_PATH + QDir::separator() + VAULT_ENCRYPY_DIR_NAME + QDir::separator() + "cryfs.config";
    if (QFile::exists(configFilePath)) {
        const QString &mountdir = VAULT_BASE_PATH + QDir::separator() + VAULT_DECRYPT_DIR_NAME;
        QStorageInfo info(mountdir);
        QString fileSystem = info.fileSystemType();
        if (info.isValid() && fileSystem == "fuse.cryfs")
            return Unlocked;
        return Encrypted;
    }

    return NotExisted;
}

// 写json文件函数
void VaultDbusResponse::ChangeJson(const QString & path, const QString & gourpNamne, const QString & VauleName, const QString & vaule)
{
    QByteArray byte;
    QFile file(path);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly|QIODevice::Text);
        byte = file.readAll();
        file.close();
    }else{
        qDebug() <<"openFileError";
        file.close();
        return;
    }

    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(byte, &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        qDebug() << " json error ";
    }

    QJsonObject rootobj = jsonDoc.object();
    if (rootobj.contains(gourpNamne)) {
        QJsonObject gourpObject = rootobj.value(gourpNamne).toObject();
        gourpObject[VauleName] = vaule;
        rootobj[gourpNamne] = gourpObject;

        if (file.exists()) {
            file.open(QIODevice::WriteOnly|QIODevice::Text);
            jsonDoc.setObject(rootobj);
            file.seek(0);
            file.write(jsonDoc.toJson());
            file.flush();
            file.close();
        }
    }
}
