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
#include "vaulthelperreceiver.h"
#include "vaulthelper_global.h"
#include "vaultconfigoperator.h"
#include "vaultassitcontrol.h"
#include "dfm-base/base/configs/dconfig/dconfigmanager.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/dpf.h>
#include <dfmio_utils.h>
#include <QDBusConnection>
#include <QDBusArgument>
#include <QFile>
#include <QStandardPaths>
#include <QProcess>

#undef signals
extern "C" {
    #include <libsecret/secret.h>
}
#define signals public

DPUTILS_USE_NAMESPACE

VaultHelperReceiver::VaultHelperReceiver(QObject *parent)
    : QObject(parent)
{

}

void VaultHelperReceiver::initEventConnect()
{
    dpfSlotChannel->connect("dfmplugin_utils", "slot_VaultHelper_ConnectLockScreenDBus",
                            this, &VaultHelperReceiver::handleConnectLockScreenDBus);
    dpfSlotChannel->connect("dfmplugin_utils", "slot_VaultHelper_TransparentUnlockVault",
                            this, &VaultHelperReceiver::handleTransparentUnlockVault);
    dpfSlotChannel->connect("dfmplugin_utils", "slot_VaultHelper_PasswordFromKeyring",
                            this, &VaultHelperReceiver::handlePasswordFromKeyring);
    dpfSlotChannel->connect("dfmplugin_utils", "slot_VaultHelper_SavePasswordToKeyring",
                            this, &VaultHelperReceiver::handleSavePasswordToKeyring);

    dpfHookSequence->follow("dfmplugin_fileoperations", "hook_Operation_MoveToTrash",
                            this, &VaultHelperReceiver::handlemoveToTrash);
}

bool VaultHelperReceiver::handleConnectLockScreenDBus()
{
    return QDBusConnection::sessionBus().connect(
                "com.deepin.SessionManager",
                "/com/deepin/SessionManager",
                "org.freedesktop.DBus.Properties",
                "PropertiesChanged", "sa{sv}as",
                this, SLOT(responseLockScreenDBus(QDBusMessage)));
}

bool VaultHelperReceiver::handleTransparentUnlockVault()
{
    return transparentUnlockVault();
}

QString VaultHelperReceiver::handlePasswordFromKeyring()
{
    qInfo() << "Vault: Read password start!";

    QString result { "" };

    GError *error = NULL;
    SecretService *service = NULL;
    char *userName = getlogin();
    qInfo() << "Vault: Get user name : " << QString(userName);
    GHashTable *attributes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    g_hash_table_insert(attributes, g_strdup("user"), g_strdup(userName));
    g_hash_table_insert(attributes, g_strdup("domain"), g_strdup("uos.cryfs"));

    service = secret_service_get_sync(SECRET_SERVICE_NONE, NULL, &error);

    SecretValue *value_read = secret_service_lookup_sync(service, NULL, attributes, NULL, &error);
    gsize length;
    const gchar* passwd = secret_value_get(value_read, &length);
    if (length > 0) {
        qInfo() << "Vault: Read password not empty!";
        result = QString(passwd);
    }

    secret_value_unref(value_read);
    g_hash_table_unref(attributes);
    g_object_unref(service);

    qWarning() << "Vault: Read password end!";

    return result;
}

bool VaultHelperReceiver::handleSavePasswordToKeyring(const QString &password)
{
    qInfo() << "Vault: start store password to keyring!";

    GError *error = NULL;
    SecretService *service = NULL;
    QByteArray baPassword = password.toLatin1();
    const char *cPassword = baPassword.data();
    // Create a password struceture
    SecretValue *value = secret_value_new_full(g_strdup(cPassword), strlen(cPassword), "text/plain", (GDestroyNotify)secret_password_free);
    // Obtain password service synchronously
    service = secret_service_get_sync(SECRET_SERVICE_NONE, NULL, &error);
    if (error == NULL) {
        GHashTable *attributes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
        // Get the currently logged in user information
        char *userName = getlogin();
        qInfo() << "Get user name : " << QString(userName);
        g_hash_table_insert(attributes, g_strdup("user"), g_strdup(userName));
        g_hash_table_insert(attributes, g_strdup("domain"), g_strdup("uos.cryfs"));
        secret_service_store_sync(service, NULL, attributes, NULL, "uos cryfs password", value, NULL, &error);
    }
    secret_value_unref(value);
    g_object_unref(value);

    if (error != NULL) {
        qWarning() << "Vault: Store password failed! error :" << QString(error->message);
        return false;
    }

    qInfo() << "Vault: Store password end!";

    return true;
}

bool VaultHelperReceiver::handlemoveToTrash(const quint64 windowId,
                                            const QList<QUrl> sources,
                                            const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags)
{
    if (sources.isEmpty())
        return false;
    if (!VaultAssitControl::instance()->isVaultFile(sources.first()))
        return false;

    QList<QUrl> localFileUrls = VaultAssitControl::instance()->transUrlsToLocal(sources);
    dpfSignalDispatcher->publish(DFMBASE_NAMESPACE::GlobalEventType::kDeleteFiles,
                                 windowId,
                                 localFileUrls, flags, nullptr);
    return true;
}

void VaultHelperReceiver::responseLockScreenDBus(const QDBusMessage &msg)
{
    const QList<QVariant> &arguments = msg.arguments();
    if (kArgumentsNum != arguments.count())
        return;

    const QString &interfaceName = msg.arguments().at(0).toString();
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

bool VaultHelperReceiver::transparentUnlockVault()
{
    VaultHelperReceiver::VaultState st = state(VaultAssitControl::instance()->vaultBaseDirLocalPath());
    if (st != VaultHelperReceiver::VaultState::kEncrypted) {
        qWarning() << "Vault: Unlock vault failed, current state is " << st;
        return false;
    }

    VaultConfigOperator config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
        const QString &passwd = handlePasswordFromKeyring();
        if (passwd.isEmpty()) {
            qWarning() << "Vault: Get password is empty, can not unlock vault!";
            return false;
        }

        const QString &mountdirPath = VaultAssitControl::instance()->vaultMountDirLocalPath();
        if (!QFile::exists(mountdirPath)) {
            QDir().mkpath(mountdirPath);
        }
        int result = unlockVault(VaultAssitControl::instance()->vaultBaseDirLocalPath(), mountdirPath, passwd);
        if (!result) {
            qInfo() << "Vault: Unlock vault success!";
            syncGroupPolicyAlgoName();
            return true;
        } else {
            if (result == 1) {
                int re = lockVault(mountdirPath, true);
                if (!re) {
                    qInfo() << "Vault: fusermount success!";
                } else {
                    qWarning() << "Vault: fusemount failed!";
                }

            }
            qWarning() << "Vault: Unlock vault failed, error code: " << result;
        }
    }
    return false;
}

VaultHelperReceiver::VaultState VaultHelperReceiver::state(const QString &encryptDir)
{
    const QString &cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        return VaultState::kNotAvailable;
    }

    QString cryfsConfigFilePath { "" };
    if (encryptDir.isEmpty()) {
        cryfsConfigFilePath = DFMIO::DFMUtils::buildFilePath(kVaultConfigPath.toStdString().c_str(),
                                                             kVaultBaseDirName,
                                                             kCryfsConfigFileName,
                                                             nullptr);
    } else {
        cryfsConfigFilePath = DFMIO::DFMUtils::buildFilePath(encryptDir.toStdString().c_str(),
                                                             kCryfsConfigFileName,
                                                             nullptr);
    }

    if (QFile::exists(cryfsConfigFilePath)) {
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(VaultAssitControl::instance()->vaultMountDirLocalPath());
        if (fsType == QString(kCryfsType)) {
            return VaultState::kUnlocked;
        }
        return VaultState::kEncrypted;
    } else {
        return VaultState::kNotExisted;
    }
}

int VaultHelperReceiver::unlockVault(const QString &basedir, const QString &mountdir, const QString &passwd)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) return -1;

    QStringList arguments;
    CryfsVersionInfo version = versionString();
    if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
        arguments << QString("--allow-replaced-filesystem");
    }
    arguments << basedir << mountdir;

    QProcess process;
    process.setEnvironment({ "CRYFS_FRONTEND=noninteractive" });
    process.start(cryfsBinary, arguments);
    process.waitForStarted();
    process.write(passwd.toUtf8());
    process.waitForBytesWritten();
    process.closeWriteChannel();
    process.waitForFinished();
    process.terminate();

    if (process.exitStatus() == QProcess::NormalExit)
        return process.exitCode();
    else
        return -1;
}

VaultHelperReceiver::CryfsVersionInfo VaultHelperReceiver::versionString()
{
    if (cryfsVersion.isVaild())
        return cryfsVersion;

    QString standardError { "" };
    QString standardOutput { "" };

    runVaultProcessAndGetOutput({ "--version" }, standardError, standardOutput);
    if (!standardOutput.isEmpty()) {
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
        QStringList &&datas = standardOutput.split('\n', QString::SkipEmptyParts);
#else
        QStringList &&datas = standardOutput.split('\n', Qt::SkipEmptyParts);
#endif
        if (!datas.isEmpty()) {
            const QString &data = datas.first();
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
            QStringList &&tmpDatas = data.split(' ', QString::SkipEmptyParts);
#else
            QStringList &&tmpDatas = data.split(' ', Qt::SkipEmptyParts);
#endif
            for (int i = 0; i < tmpDatas.size(); ++i) {
                if (tmpDatas.at(i).contains(QRegExp("^[0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}$"))) {
                    const QString tmpVersions = tmpDatas.at(i);
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
                    QStringList &&versions = tmpVersions.split('.', QString::SkipEmptyParts);
#else
                    QStringList &&versions = tmpVersions.split('.', Qt::SkipEmptyParts);
#endif
                    cryfsVersion.majorVersion = versions.at(kMajorIndex).toInt();
                    cryfsVersion.minorVersion = versions.at(kMinorIndex).toInt();
                    cryfsVersion.hotfixVersion = versions.at(kHotFixIndex).toInt();
                    break;
                }
            }
        }
    }

    return cryfsVersion;
}

void VaultHelperReceiver::runVaultProcessAndGetOutput(const QStringList &arguments, QString &standardError, QString &standardOutput)
{
    const QString &cryfsProgram = QStandardPaths::findExecutable("cryfs");
    if (cryfsProgram.isEmpty()) {
        qWarning() << "cryfs is not exist!";
        return;
    }

    QProcess process;
    process.setEnvironment({ "CRYFS_FRONTEND=noninteractive", "CRYFS_NO_UPDATE_CHECK=true" });
    process.start(cryfsProgram, arguments);
    process.waitForStarted();
    process.waitForFinished();
    standardError = QString::fromLocal8Bit(process.readAllStandardError());
    standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
}

void VaultHelperReceiver::syncGroupPolicyAlgoName()
{
    VaultConfigOperator config;
    const QString &algoName = config.get(kConfigNodeName, kConfigKeyAlgoName, QVariant(kConfigKeyNotExist)).toString();
    if (algoName == QString(kConfigKeyNotExist)) {
        dfmbase::DConfigManager::instance()->setValue( dfmbase::kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, QVariant("aes-256-gcm"));
    } else {
        if (!algoName.isEmpty())
            dfmbase::DConfigManager::instance()->setValue( dfmbase::kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, algoName);
    }
}

int VaultHelperReceiver::lockVault(QString unlockFileDir, bool isForced)
{
    CryfsVersionInfo version = versionString();
    QString fusermountBinary;
    QStringList arguments;
    qInfo() << QString("Vault: cryfs version is %1.%2.%3").arg(version.majorVersion).arg(version.minorVersion).arg(version.hotfixVersion);
    if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
        fusermountBinary = QStandardPaths::findExecutable("cryfs-unmount");
        arguments << unlockFileDir;
    } else {
        fusermountBinary = QStandardPaths::findExecutable("fusermount");
        if (isForced) {
            arguments << "-zu" << unlockFileDir;
        } else {
            arguments << "-u" << unlockFileDir;
        }
    }
    if (fusermountBinary.isEmpty()) return -1;

    QProcess process;
    process.start(fusermountBinary, arguments);
    process.waitForStarted();
    process.waitForFinished();
    process.terminate();

    if (process.exitStatus() == QProcess::NormalExit)
        return process.exitCode();
    else
        return -1;
}
