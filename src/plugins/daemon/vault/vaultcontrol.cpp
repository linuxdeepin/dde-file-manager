// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultcontrol.h"
#include "vaulthelper.h"
#include "vaultconfigoperator.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/dfm_event_defines.h>

#include <dfm-io/dfmio_utils.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusArgument>
#include <QStandardPaths>
#include <QProcess>
#include <QFile>
#include <QDateTime>
#include <QCoreApplication>
#include <QRegularExpression>

extern "C" {
#include <libsecret/secret.h>
}

inline constexpr int kArgumentsNum { 3 };
using namespace GlobalDConfDefines::ConfigPath;

DAEMONPVAULT_USE_NAMESPACE

VaultControl *VaultControl::instance()
{
    static VaultControl ins;
    return &ins;
}

void VaultControl::connectLockScreenDBus()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        fmCritical() << "[VaultControl::connectLockScreenDBus] Failed to connect to D-Bus session bus";
        return;
    }

    if (!connection.interface()->isServiceRegistered(kAppSessionService)) {
        fmCritical() << "[VaultControl::connectLockScreenDBus] DBus service not registered:" << kAppSessionService;
        return;
    }

    if (!QDBusConnection::sessionBus().connect(kAppSessionService, kAppSessionPath, "org.freedesktop.DBus.Properties",
                                               "PropertiesChanged", "sa{sv}as", this, SLOT(responseLockScreenDBus(QDBusMessage)))) {
        fmCritical() << "[VaultControl::connectLockScreenDBus] Failed to connect to lock screen DBus signal";
    } else {
        fmInfo() << "[VaultControl::connectLockScreenDBus] Successfully connected to lock screen DBus";
    }
}

void VaultControl::responseLockScreenDBus(const QDBusMessage &msg)
{
    const QList<QVariant> &arguments = msg.arguments();
    if (kArgumentsNum != arguments.count()) {
        fmWarning() << "[VaultControl::responseLockScreenDBus] Invalid DBus message arguments count:" << arguments.count() << "expected:" << kArgumentsNum;
        return;
    }

    const QString &interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != kAppSessionService)
        return;

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    Q_FOREACH (const QString &prop, keys) {
        if (prop == "Locked") {
            bool isLocked = changedProps[prop].toBool();
            if (!isLocked) {
                fmInfo() << "[VaultControl::responseLockScreenDBus] Screen unlocked, attempting transparent vault unlock";
                transparentUnlockVault();
            } else {
                fmInfo() << "[VaultControl::responseLockScreenDBus] Screen locked, locking vault";
                lockVault(VaultHelper::instance()->vaultMountDirLocalPath(), false);

                QString filePath { QDir::homePath() + QString("/.config/%1/dde-file-manager/").arg(qApp->organizationName()) + kVaultTimeConfigFileName };
                DFMBASE_NAMESPACE::Settings setting("", "", filePath);
                setting.setValue(QString("VaultTime"), QString("LockTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                fmInfo() << "[VaultControl::responseLockScreenDBus] Vault lock time recorded";
            }
        }
    }
}

bool VaultControl::transparentUnlockVault()
{
    VaultState st = state(VaultHelper::instance()->vaultBaseDirLocalPath());
    if (st != kEncrypted) {
        fmInfo() << "[VaultControl::transparentUnlockVault] Vault not in encrypted state, current state:" << static_cast<int>(st);
        return false;
    }

    VaultConfigOperator config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
        fmInfo() << "[VaultControl::transparentUnlockVault] Attempting transparent unlock";
        const QString &passwd = passwordFromKeyring();
        if (passwd.isEmpty()) {
            fmWarning() << "[VaultControl::transparentUnlockVault] Failed to retrieve password from keyring";
            return false;
        }

        const QString &mountdirPath = VaultHelper::instance()->vaultMountDirLocalPath();
        if (!QFile::exists(mountdirPath)) {
            QDir().mkpath(mountdirPath);
            fmInfo() << "[VaultControl::transparentUnlockVault] Created mount directory:" << mountdirPath;
        }
        int result = unlockVault(VaultHelper::instance()->vaultBaseDirLocalPath(), mountdirPath, passwd);
        if (!result) {
            fmInfo() << "[VaultControl::transparentUnlockVault] Vault unlocked successfully";
            syncGroupPolicyAlgoName();
            return true;
        } else {
            if (result == 1) {
                int re = lockVault(mountdirPath, false);
                if (!re) {
                    fmInfo() << "[VaultControl::transparentUnlockVault] Cleanup: fusermount successful";
                } else {
                    fmWarning() << "[VaultControl::transparentUnlockVault] Cleanup: fusermount failed with code:" << re;
                }
            }
            fmWarning() << "[VaultControl::transparentUnlockVault] Failed to unlock vault, error code:" << result;
        }
    } else {
        fmInfo() << "[VaultControl::transparentUnlockVault] Vault not configured for transparent unlock, method:" << encryptionMethod;
    }
    return false;
}

void VaultControl::MonitorNetworkStatus()
{
    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.isConnected()) {
        fmCritical() << "[VaultControl::MonitorNetworkStatus] Failed to connect to D-Bus system bus";
        return;
    }

    if (!connection.interface()->isServiceRegistered(kNetWorkDBusServiceName)) {
        fmCritical() << "[VaultControl::MonitorNetworkStatus] Network DBus service not registered:" << kNetWorkDBusServiceName;
        return;
    }

    if (!QDBusConnection::systemBus().connect(kNetWorkDBusServiceName, kNetWorkDBusPath, kNetWorkDBusInterfaces,
                                              "ConnectivityChanged", this, SLOT(responseNetworkStateChaneDBus(int)))) {
        fmCritical() << "[VaultControl::MonitorNetworkStatus] Failed to connect to network connectivity signal";
    } else {
        fmInfo() << "[VaultControl::MonitorNetworkStatus] Successfully connected to network monitoring";
    }
}

void VaultControl::responseNetworkStateChaneDBus(int st)
{
    Connectivity enState = static_cast<Connectivity>(st);
    fmInfo() << "[VaultControl::responseNetworkStateChaneDBus] Network connectivity changed to:" << static_cast<int>(enState);
    
    if (enState == Connectivity::Full) {
        VaultConfigOperator config;
        QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
        if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
            fmInfo() << "[VaultControl::responseNetworkStateChaneDBus] Transparent method detected, no action needed";
            return;
        } else if (encryptionMethod == QString(kConfigValueMethodKey) || encryptionMethod == QString(kConfigKeyNotExist)) {
            VaultState st = state(VaultHelper::instance()->vaultBaseDirLocalPath());
            if (st != kUnlocked) {
                fmInfo() << "[VaultControl::responseNetworkStateChaneDBus] Vault not unlocked, current state:" << static_cast<int>(st);
                return;
            }

            fmInfo() << "[VaultControl::responseNetworkStateChaneDBus] Network connected, locking vault for security";
            const QString mountPath = VaultHelper::instance()->vaultMountDirLocalPath();
            int re = lockVault(mountPath, false);
            if (re == 0) {
                QVariantMap map;
                map.insert(mountPath, QVariant::fromValue(static_cast<int>(VaultState::kEncrypted)));
                Q_EMIT changedVaultState(map);
                fmInfo() << "[VaultControl::responseNetworkStateChaneDBus] Vault locked successfully due to network connection";
            } else {
                fmWarning() << "[VaultControl::responseNetworkStateChaneDBus] Failed to lock vault, error code:" << re;
            }
        }
    }
}

VaultControl::CryfsVersionInfo VaultControl::versionString()
{
    if (cryfsVersion.isVaild()) {
        fmDebug() << "[VaultControl::versionString] Using cached version:" << cryfsVersion.majorVersion << "." << cryfsVersion.minorVersion << "." << cryfsVersion.hotfixVersion;
        return cryfsVersion;
    }

    fmDebug() << "[VaultControl::versionString] Retrieving cryfs version information";
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
                if (tmpDatas.at(i).contains(QRegularExpression("^[0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}$"))) {
                    const QString tmpVersions = tmpDatas.at(i);
#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
                    QStringList &&versions = tmpVersions.split('.', QString::SkipEmptyParts);
#else
                    QStringList &&versions = tmpVersions.split('.', Qt::SkipEmptyParts);
#endif
                    cryfsVersion.majorVersion = versions.at(kMajorIndex).toInt();
                    cryfsVersion.minorVersion = versions.at(kMinorIndex).toInt();
                    cryfsVersion.hotfixVersion = versions.at(kHotFixIndex).toInt();
                    fmInfo() << "[VaultControl::versionString] Detected cryfs version:" << cryfsVersion.majorVersion << "." << cryfsVersion.minorVersion << "." << cryfsVersion.hotfixVersion;
                    break;
                }
            }
        }
    } else {
        fmWarning() << "[VaultControl::versionString] Failed to get cryfs version, stderr:" << standardError;
    }

    return cryfsVersion;
}

void VaultControl::runVaultProcessAndGetOutput(const QStringList &arguments, QString &standardError, QString &standardOutput)
{
    const QString &cryfsProgram = QStandardPaths::findExecutable("cryfs");
    if (cryfsProgram.isEmpty()) {
        fmCritical() << "[VaultControl::runVaultProcessAndGetOutput] cryfs executable not found in PATH";
        return;
    }

    fmDebug() << "[VaultControl::runVaultProcessAndGetOutput] Executing cryfs with arguments:" << arguments;
    QProcess process;
    process.setEnvironment({ "CRYFS_FRONTEND=noninteractive", "CRYFS_NO_UPDATE_CHECK=true" });
    process.start(cryfsProgram, arguments);
    process.waitForStarted();
    process.waitForFinished();
    standardError = QString::fromLocal8Bit(process.readAllStandardError());
    standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
}

VaultState VaultControl::state(const QString &encryptDir)
{
    const QString &cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        fmWarning() << "[VaultControl::state] cryfs binary not found, vault not available";
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

    fmDebug() << "[VaultControl::state] Checking vault state, config file:" << cryfsConfigFilePath;

    if (QFile::exists(cryfsConfigFilePath)) {
        QUrl mountPonitUrl = QUrl::fromLocalFile(VaultHelper::instance()->vaultMountDirLocalPath());
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(mountPonitUrl);
        if (fsType == QString(kCryfsType)) {
            fmDebug() << "[VaultControl::state] Vault is unlocked, filesystem type:" << fsType;
            return VaultState::kUnlocked;
        }
        fmDebug() << "[VaultControl::state] Vault is encrypted, filesystem type:" << fsType;
        return VaultState::kEncrypted;
    } else {
        fmDebug() << "[VaultControl::state] Vault does not exist, config file not found";
        return VaultState::kNotExisted;
    }
}

void VaultControl::syncGroupPolicyAlgoName()
{
    VaultConfigOperator config;
    const QString &algoName = config.get(kConfigNodeName, kConfigKeyAlgoName, QVariant(kConfigKeyNotExist)).toString();
    if (algoName == QString(kConfigKeyNotExist)) {
        dfmbase::DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, QVariant("aes-256-gcm"));
        fmInfo() << "[VaultControl::syncGroupPolicyAlgoName] Set default algorithm: aes-256-gcm";
    } else {
        if (!algoName.isEmpty()) {
            dfmbase::DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, algoName);
            fmInfo() << "[VaultControl::syncGroupPolicyAlgoName] Synced algorithm from config:" << algoName;
        }
    }
}

QString VaultControl::passwordFromKeyring()
{
    fmDebug() << "[VaultControl::passwordFromKeyring] Retrieving password from keyring";

    QString result { "" };

    GError *error = NULL;
    SecretService *service = NULL;
    char *userName = getlogin();
    fmDebug() << "[VaultControl::passwordFromKeyring] Current user:" << QString(userName);
    GHashTable *attributes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    g_hash_table_insert(attributes, g_strdup("user"), g_strdup(userName));
    g_hash_table_insert(attributes, g_strdup("domain"), g_strdup("uos.cryfs"));

    service = secret_service_get_sync(SECRET_SERVICE_NONE, NULL, &error);

    SecretValue *value_read = secret_service_lookup_sync(service, NULL, attributes, NULL, &error);
    gsize length;
    const gchar *passwd = secret_value_get(value_read, &length);
    if (length > 0) {
        fmDebug() << "[VaultControl::passwordFromKeyring] Password retrieved successfully";
        result = QString(passwd);
    } else {
        fmWarning() << "[VaultControl::passwordFromKeyring] No password found in keyring";
    }

    secret_value_unref(value_read);
    g_hash_table_destroy(attributes);

    return result;
}

int VaultControl::unlockVault(const QString &basedir, const QString &mountdir, const QString &passwd)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        fmCritical() << "[VaultControl::unlockVault] cryfs binary not found";
        return -1;
    }

    fmInfo() << "[VaultControl::unlockVault] Attempting to unlock vault from:" << basedir << "to:" << mountdir;

    QStringList arguments;
    CryfsVersionInfo version = versionString();
    if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
        arguments << QString("--allow-replaced-filesystem");
        fmDebug() << "[VaultControl::unlockVault] Using --allow-replaced-filesystem flag for cryfs version >= 0.10.0";
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

    int exitCode = -1;
    if (process.exitStatus() == QProcess::NormalExit) {
        exitCode = process.exitCode();
        if (exitCode == 0) {
            fmInfo() << "[VaultControl::unlockVault] Vault unlocked successfully";
        } else {
            fmWarning() << "[VaultControl::unlockVault] Vault unlock failed with exit code:" << exitCode;
        }
    } else {
        fmCritical() << "[VaultControl::unlockVault] cryfs process crashed or was terminated abnormally";
    }

    return exitCode;
}

int VaultControl::lockVault(const QString &unlockFileDir, bool isForced)
{
    /*
     * NOTE: When unmount the vault using the cryfs-unmount command,
     * the valut will be unmounted normally even if it is busy (file copy)
     */
    // CryfsVersionInfo version = versionString();
    // if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
    //     fusermountBinary = QStandardPaths::findExecutable("cryfs-unmount");
    //     arguments << unlockFileDir;
    // } else {
    QString fusermountBinary;
    QStringList arguments;
    fusermountBinary = QStandardPaths::findExecutable("fusermount");
    if (isForced) {
        arguments << "-zu" << unlockFileDir;
        fmInfo() << "[VaultControl::lockVault] Force unmounting vault at:" << unlockFileDir;
    } else {
        arguments << "-u" << unlockFileDir;
        fmInfo() << "[VaultControl::lockVault] Unmounting vault at:" << unlockFileDir;
    }
    if (fusermountBinary.isEmpty()) {
        fmCritical() << "[VaultControl::lockVault] fusermount binary not found";
        return -1;
    }

    QProcess process;
    process.start(fusermountBinary, arguments);
    process.waitForStarted();
    process.waitForFinished();
    process.terminate();

    int exitCode = -1;
    if (process.exitStatus() == QProcess::NormalExit) {
        exitCode = process.exitCode();
        if (exitCode == 0) {
            fmInfo() << "[VaultControl::lockVault] Vault locked successfully";
        } else {
            fmWarning() << "[VaultControl::lockVault] Vault lock failed with exit code:" << exitCode;
        }
    } else {
        fmCritical() << "[VaultControl::lockVault] fusermount process crashed or was terminated abnormally";
    }

    return exitCode;
}

VaultControl::VaultControl(QObject *parent)
    : QObject(parent)
{
}
