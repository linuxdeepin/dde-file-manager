// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
#include <QDebug>
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
        fmWarning() << "Vault Daemon: Cannot connect to the D-Bus session bus.";
        return;
    }

    if (!connection.interface()->isServiceRegistered(kAppSessionService)) {
        fmCritical("Vault Daemon: Cannot register the \"org.deepin.Filemanager.Daemon\" service!!!\n");
        return;
    }

    if (!QDBusConnection::sessionBus().connect(kAppSessionService, kAppSessionPath, "org.freedesktop.DBus.Properties",
                                               "PropertiesChanged", "sa{sv}as", this, SLOT(responseLockScreenDBus(QDBusMessage)))) {
        fmCritical() << "Vault Daemon: Vault Server Error: connect lock screen dbus error!";
    }
}

void VaultControl::responseLockScreenDBus(const QDBusMessage &msg)
{
    const QList<QVariant> &arguments = msg.arguments();
    if (kArgumentsNum != arguments.count()) {
        fmCritical() << "Vault Daemon: Vault Server Error: arguments of lock screen dbus error!";
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
                transparentUnlockVault();
            } else {
                lockVault(VaultHelper::instance()->vaultMountDirLocalPath(), false);

                QString filePath { QDir::homePath() + QString("/.config/%1/dde-file-manager/").arg(qApp->organizationName()) + kVaultTimeConfigFileName };
                DFMBASE_NAMESPACE::Settings setting("", "", filePath);
                setting.setValue(QString("VaultTime"), QString("LockTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            }
        }
    }
}

bool VaultControl::transparentUnlockVault()
{
    VaultState st = state(VaultHelper::instance()->vaultBaseDirLocalPath());
    if (st != kEncrypted) {
        fmWarning() << "Vault Daemon: Unlock vault failed, current state is " << st;
        return false;
    }

    VaultConfigOperator config;
    QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
    if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
        const QString &passwd = passwordFromKeyring();
        if (passwd.isEmpty()) {
            fmWarning() << "Vault Daemon: Get password is empty, can not unlock vault!";
            return false;
        }

        const QString &mountdirPath = VaultHelper::instance()->vaultMountDirLocalPath();
        if (!QFile::exists(mountdirPath)) {
            QDir().mkpath(mountdirPath);
        }
        int result = unlockVault(VaultHelper::instance()->vaultBaseDirLocalPath(), mountdirPath, passwd);
        if (!result) {
            fmInfo() << "Vault Daemon: Unlock vault success!";
            syncGroupPolicyAlgoName();
            return true;
        } else {
            if (result == 1) {
                int re = lockVault(mountdirPath, false);
                if (!re) {
                    fmInfo() << "Vault Daemon: fusermount success!";
                } else {
                    fmWarning() << "Vault Daemon: fusemount failed!";
                }
            }
            fmWarning() << "Vault Daemon: Unlock vault failed, error code: " << result;
        }
    }
    return false;
}

void VaultControl::MonitorNetworkStatus()
{
    QDBusConnection connection = QDBusConnection::systemBus();
    if (!connection.isConnected()) {
        fmWarning() << "Cannot connect to the D-Bus system bus.";
        return;
    }

    if (!connection.interface()->isServiceRegistered(kNetWorkDBusServiceName)) {
        fmCritical() << "Not register the service !" << kNetWorkDBusServiceName;
        return;
    }

    if (!QDBusConnection::systemBus().connect(kNetWorkDBusServiceName, kNetWorkDBusPath, kNetWorkDBusInterfaces,
                                              "ConnectivityChanged", this, SLOT(responseNetworkStateChaneDBus(int)))) {
        fmCritical() << "Connect network dbus error!";
    }
}

void VaultControl::responseNetworkStateChaneDBus(int st)
{
    Connectivity enState = static_cast<Connectivity>(st);
    if (enState == Connectivity::Full) {
        VaultConfigOperator config;
        QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
        if (encryptionMethod == QString(kConfigValueMethodTransparent)) {
            return;
        } else if (encryptionMethod == QString(kConfigValueMethodKey) || encryptionMethod == QString(kConfigKeyNotExist)) {
            VaultState st = state(VaultHelper::instance()->vaultBaseDirLocalPath());
            if (st != kUnlocked)
                return;

            const QString mountPath = VaultHelper::instance()->vaultMountDirLocalPath();
            int re = lockVault(mountPath, false);
            if (re == 0) {
                QVariantMap map;
                map.insert(mountPath, QVariant::fromValue(static_cast<int>(VaultState::kEncrypted)));
                Q_EMIT changedVaultState(map);
            }
        }
    }
}

VaultControl::CryfsVersionInfo VaultControl::versionString()
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
                    break;
                }
            }
        }
    }

    return cryfsVersion;
}

void VaultControl::runVaultProcessAndGetOutput(const QStringList &arguments, QString &standardError, QString &standardOutput)
{
    const QString &cryfsProgram = QStandardPaths::findExecutable("cryfs");
    if (cryfsProgram.isEmpty()) {
        fmWarning() << "Vault Daemon: cryfs is not exist!";
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

VaultState VaultControl::state(const QString &encryptDir)
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
        QUrl mountPonitUrl = QUrl::fromLocalFile(VaultHelper::instance()->vaultMountDirLocalPath());
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(mountPonitUrl);
        if (fsType == QString(kCryfsType)) {
            return VaultState::kUnlocked;
        }
        return VaultState::kEncrypted;
    } else {
        return VaultState::kNotExisted;
    }
}

void VaultControl::syncGroupPolicyAlgoName()
{
    VaultConfigOperator config;
    const QString &algoName = config.get(kConfigNodeName, kConfigKeyAlgoName, QVariant(kConfigKeyNotExist)).toString();
    if (algoName == QString(kConfigKeyNotExist)) {
        dfmbase::DConfigManager::instance()->setValue(dfmbase::kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, QVariant("aes-256-gcm"));
    } else {
        if (!algoName.isEmpty())
            dfmbase::DConfigManager::instance()->setValue(dfmbase::kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, algoName);
    }
}

QString VaultControl::passwordFromKeyring()
{
    fmInfo() << "Vault Daemon: Read password start!";

    QString result { "" };

    GError *error = NULL;
    SecretService *service = NULL;
    char *userName = getlogin();
    fmInfo() << "Vault: Get user name : " << QString(userName);
    GHashTable *attributes = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    g_hash_table_insert(attributes, g_strdup("user"), g_strdup(userName));
    g_hash_table_insert(attributes, g_strdup("domain"), g_strdup("uos.cryfs"));

    service = secret_service_get_sync(SECRET_SERVICE_NONE, NULL, &error);

    SecretValue *value_read = secret_service_lookup_sync(service, NULL, attributes, NULL, &error);
    gsize length;
    const gchar *passwd = secret_value_get(value_read, &length);
    if (length > 0) {
        fmInfo() << "Vault Daemon: Read password not empty!";
        result = QString(passwd);
    }

    secret_value_unref(value_read);
    g_hash_table_unref(attributes);
    g_object_unref(service);

    fmWarning() << "Vault Daemon: Read password end!";

    return result;
}

int VaultControl::unlockVault(const QString &basedir, const QString &mountdir, const QString &passwd)
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

int VaultControl::lockVault(const QString &unlockFileDir, bool isForced)
{
    CryfsVersionInfo version = versionString();
    QString fusermountBinary;
    QStringList arguments;
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

VaultControl::VaultControl(QObject *parent)
    : QObject(parent)
{
}
