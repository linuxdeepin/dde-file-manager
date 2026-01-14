// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileencrypthandle.h"
#include "fileencrypthandle_p.h"
#include "encryption/vaultconfig.h"
#include "encryption/passwordmanager.h"
#include "encryption/masterkeymanager.h"
#include "encryption/operatorcenter.h"
#include "pathmanager.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-io/dfmio_utils.h>

#include <QDirIterator>
#include <QStandardPaths>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QDateTime>
#include <QUrl>
#include <QDBusConnectionInterface>

#include <unistd.h>

inline constexpr int kArgumentsNum { 3 };

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;
using namespace GlobalDConfDefines::ConfigPath;
DCORE_USE_NAMESPACE

FileEncryptHandle::FileEncryptHandle(QObject *parent)
    : QObject(parent), d(new FileEncryptHandlerPrivate(this))
{
    fmDebug() << "Vault: Initializing FileEncryptHandle";
    connect(d->process, &QProcess::readyReadStandardError, this, &FileEncryptHandle::slotReadError);
    connect(d->process, &QProcess::readyReadStandardOutput, this, &FileEncryptHandle::slotReadOutput);
    fmDebug() << "Vault: FileEncryptHandle initialization completed";
}

FileEncryptHandle::~FileEncryptHandle()
{
    fmDebug() << "Vault: Destroying FileEncryptHandle";
    disconnect(d->process, &QProcess::readyReadStandardError, this, &FileEncryptHandle::slotReadError);
    disconnect(d->process, &QProcess::readyReadStandardOutput, this, &FileEncryptHandle::slotReadOutput);

    delete d;
    d = nullptr;
    fmDebug() << "Vault: FileEncryptHandle destroyed";
}

FileEncryptHandle *FileEncryptHandle::instance()
{
    static FileEncryptHandle ins;
    return &ins;
}

/*!
 * \brief                      创建保险箱
 * \param[in] lockBaseDir:     保险箱加密文件夹
 * \param[in] unlockFileDir:   保险箱解密文件夹
 * \param[in] passWord:        保险箱密码
 * \param[in] type:            加密类型
 * \param[in] blockSize:       解密文件块大小(大小影响加密文件的多少以及效率)
 * \note
 *  使用map容器activeState的key为1来记录创建状态标记，
 *  调用runVaultProcess函数进行创建保险箱并返回创建状态标记。
 *  最后使用信号signalCreateVault发送创建状态标记。
 */
void FileEncryptHandle::createVault(const QString &lockBaseDir, const QString &unlockFileDir,
                                    const QString &passWord, EncryptType type, int blockSize)
{
    fmInfo() << "Vault: Starting vault creation";
    if (!(createDirIfNotExist(lockBaseDir) && createDirIfNotExist(unlockFileDir))) {
        fmCritical() << "Vault: Failed to create required directories for vault";
        return;
    }

    d->mutex->lock();
    d->activeState.insert(1, static_cast<int>(ErrorCode::kSuccess));

    QString cryfsConfigPath = lockBaseDir;
    if (!cryfsConfigPath.endsWith("/"))
        cryfsConfigPath += "/";
    cryfsConfigPath += kCryfsConfigFileName;

    const QString &algoName = d->encryptTypeMap.value(type);
    DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, algoName);
    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyAlgoName, QVariant(algoName));

    if (!QFile::exists(cryfsConfigPath)) {
        DSecureString cryfsPwd = DSecureString(passWord);
        QString encryptionMethod = config.get(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(kConfigKeyNotExist)).toString();
        if (encryptionMethod == QString(kConfigValueMethodKey)) {
            QByteArray masterKey = MasterKeyManager::generateMasterKey();
            if (masterKey.isEmpty()) {
                d->activeState[1] = static_cast<int>(ErrorCode::kUnspecifiedError);
                emit signalCreateVault(d->activeState.value(1));
                fmWarning() << "Vault: Failed to generate master key";
                d->activeState.clear();
                d->mutex->unlock();
                return;
            }

            QString containerPath = MasterKeyManager::getContainerPath();
            int ret = PasswordManager::createPasswordContainerFile(containerPath.toUtf8().constData());
            if (ret != 0) {
                d->activeState[1] = static_cast<int>(ErrorCode::kUnspecifiedError);
                emit signalCreateVault(d->activeState.value(1));
                fmCritical() << "Vault: Failed to create password container file";
                d->activeState.clear();
                d->mutex->unlock();
                return;
            }

            int slotID = 0;
            ret = PasswordManager::createLuksContainer(containerPath.toUtf8().constData(),
                                                        masterKey.data(), masterKey.size(),
                                                        passWord.toUtf8().constData(), slotID);
            if (ret != 0) {
                d->activeState[1] = static_cast<int>(ErrorCode::kUnspecifiedError);
                emit signalCreateVault(d->activeState.value(1));
                fmCritical() << "Vault: Failed to create LUKS container";
                d->activeState.clear();
                d->mutex->unlock();
                return;
            }

            QString preGeneratedRecoveryKey = OperatorCenter::getInstance()->getRecoveryKey();
            char recoveryKey[33];
            if (!preGeneratedRecoveryKey.isEmpty() && preGeneratedRecoveryKey.length() == 32) {
                QByteArray recoveryKeyBytes = preGeneratedRecoveryKey.toUtf8();
                memcpy(recoveryKey, recoveryKeyBytes.data(), 32);
                recoveryKey[32] = '\0';
            } else {
                d->activeState[1] = static_cast<int>(ErrorCode::kUnspecifiedError);
                emit signalCreateVault(d->activeState.value(1));
                fmCritical() << "Vault: Failed to gett recovery key";
                d->activeState.clear();
                d->mutex->unlock();
                return;
            }
            int recoveryKeySlotID = 0;
            ret = PasswordManager::addNewPassword(containerPath.toUtf8().constData(),
                                                 passWord.toUtf8().constData(),
                                                 recoveryKey, recoveryKeySlotID);
            if (ret != 0) {
                d->activeState[1] = static_cast<int>(ErrorCode::kUnspecifiedError);
                emit signalCreateVault(d->activeState.value(1));
                fmCritical() << "Vault: Failed to add recovery key";
                d->activeState.clear();
                d->mutex->unlock();
                return;
            }
            cryfsPwd = DSecureString::fromUtf8(masterKey);
        }

        int flg = d->runVaultProcess(lockBaseDir, unlockFileDir, cryfsPwd, type, blockSize);
        if (d->activeState.value(1) != static_cast<int>(ErrorCode::kSuccess)) {
            emit signalCreateVault(d->activeState.value(1));
            fmCritical() << "Vault: create vault failed!";
        } else {
            config.setVaultCreationType(kConfigValueVaultCreationTypeNew);
            d->curState = kUnlocked;
            emit signalCreateVault(flg);
        }
    } else {
        fmWarning() << "Vault: Vault already exists when creating, this should not happen normally";
        d->activeState[1] = static_cast<int>(ErrorCode::kUnspecifiedError);
        emit signalCreateVault(d->activeState.value(1));
        fmWarning() << "Vault: Cannot create vault, vault already exists";
    }
    d->activeState.clear();
    d->mutex->unlock();
}

/*!
 * \brief                       解锁保险箱
 * \param[in] lockBaseDir:      保险箱加密文件夹
 * \param[in] unlockFileDir:    保险箱解密文件夹
 * \param[in] passWord:         保险箱密码
 * \note
 *  使用map容器activeState的key为3来记录解锁状态标记，
 *  调用runVaultProcess函数进行解锁保险箱并返回解锁状态标记。
 *  最后使用信号signalUnlockVault发送解锁状态标记。
 */
bool FileEncryptHandle::unlockVault(const QString &lockBaseDir, const QString &unlockFileDir, const QString &DSecureString)
{
    fmInfo() << "Vault: Starting vault unlock";
    if (!createDirIfNotExist(unlockFileDir)) {
        fmCritical() << "Vault: Failed to create unlock directory:" << unlockFileDir;
        DialogManager::instance()->showErrorDialog(tr("Unlock failed"), tr("The %1 directory is occupied,\n please clear the files in this directory and try to unlock the safe again.").arg(unlockFileDir));
        return false;
    }

    bool result { false };
    d->mutex->lock();
    d->activeState.insert(3, static_cast<int>(ErrorCode::kSuccess));
    d->syncGroupPolicyAlgoName();

    // 检测保险箱版本（通过检查 password_container.bin 文件是否存在）
    OperatorCenter *operatorCenter = OperatorCenter::getInstance();
    bool isNewVersion = operatorCenter->isNewVaultVersion();

    if (isNewVersion) {
        // 新版本：从LUKS容器获取主密钥
        fmInfo() << "Vault: Unlocking new version vault";

        QString containerPath = MasterKeyManager::getContainerPath();
        QString inputPassword = DSecureString;

        QByteArray cryfsPassword;
        bool isMasterKey = false;

        // 尝试Base64解码，检查是否是主密钥
        // checkPassword返回的是Base64编码的主密钥（已经根据创建方式处理过的）
        QByteArray decoded = QByteArray::fromBase64(inputPassword.toLatin1());
        if (decoded.size() == 64 || (decoded.size() > 0 && decoded.size() < 64)) {
            // 是Base64编码的主密钥，直接使用
            cryfsPassword = decoded;
            isMasterKey = true;
            fmInfo() << "Vault: Detected master key (Base64 encoded), using directly for CryFS";
        } else {
            // 是用户密码，需要从LUKS容器获取主密钥
            fmInfo() << "Vault: Detected user password, extracting master key from LUKS container";

            // 1. 验证密码
            bool isValid = false;
            int ret = PasswordManager::verifyPassword(containerPath.toUtf8().constData(),
                                                      inputPassword.toUtf8().constData(), isValid);
            if (ret != 0 || !isValid) {
                result = false;
                d->activeState[3] = static_cast<int>(ErrorCode::kWrongPassword);
                emit signalUnlockVault(d->activeState.value(3));
                fmWarning() << "Vault: Password verification failed";
                d->activeState.clear();
                d->mutex->unlock();
                return result;
            }

            // 2. 从LUKS容器导出主密钥
            char masterKeyBuf[64];
            size_t masterKeySize = 64;
            ret = PasswordManager::exportMasterKey(containerPath.toUtf8().constData(),
                                                   inputPassword.toUtf8().constData(),
                                                   masterKeyBuf, &masterKeySize);
            if (ret != 0) {
                result = false;
                d->activeState[3] = static_cast<int>(ErrorCode::kUnspecifiedError);
                emit signalUnlockVault(d->activeState.value(3));
                fmWarning() << "Vault: Failed to export master key";
                d->activeState.clear();
                d->mutex->unlock();
                return result;
            }

            QByteArray masterKey = QByteArray(masterKeyBuf, 64);

            // 3. 根据创建方式处理主密钥
            VaultConfig config;
            QString creationType = config.getVaultCreationType();

            if (creationType == kConfigValueVaultCreationTypeNew) {
                // 新创建的保险箱：主密钥是64字节随机数，直接使用
                cryfsPassword = masterKey;
            } else if (creationType == kConfigValueVaultCreationTypeMigrated) {
                // 迁移的保险箱：主密钥是旧密码+补零，需要去除末尾零还原原始密码
                cryfsPassword = masterKey;
                while (cryfsPassword.endsWith('\0')) {
                    cryfsPassword.chop(1);
                }
            } else {
                // 兼容处理：如果没有设置创建方式，默认按新创建处理
                cryfsPassword = masterKey;
            }
        }

        // 4. 使用处理后的密码解锁CryFS
        Dtk::Core::DSecureString cryfsPasswordSecure = Dtk::Core::DSecureString::fromUtf8(cryfsPassword);
        int flg = d->runVaultProcess(lockBaseDir, unlockFileDir, cryfsPasswordSecure);
        if (d->activeState.value(3) != static_cast<int>(ErrorCode::kSuccess)) {
            result = false;
            emit signalUnlockVault(d->activeState.value(3));
            fmWarning() << "Vault: Unlock vault failed with error code:" << d->activeState.value(3);
        } else {
            result = true;
            d->curState = kUnlocked;
            emit signalUnlockVault(flg);
            fmInfo() << "Vault: unlock vault success!";
        }
    } else {
        // 旧版本保险箱：使用旧方法解锁（没有LUKS容器，直接使用用户密码或pbkdf2加密后的密码）
        fmInfo() << "Vault: Unlocking old version vault (created by old software)";
        int flg = d->runVaultProcess(lockBaseDir, unlockFileDir, DSecureString);
        if (d->activeState.value(3) != static_cast<int>(ErrorCode::kSuccess)) {
            result = false;
            emit signalUnlockVault(d->activeState.value(3));
            fmWarning() << "Vault: Unlock vault failed with error code:" << d->activeState.value(3);
        } else {
            result = true;
            d->curState = kUnlocked;
            emit signalUnlockVault(flg);
            fmInfo() << "Vault: unlock vault success!";
        }
    }
    d->activeState.clear();
    d->mutex->unlock();

    return result;
}

/*!
 * \brief                        加锁保险箱
 * \param[in] unlockFileDir:     保险箱解密文件夹
 * \note
 *  使用map容器activeState的key为7来记录上锁状态标记，
 *  调用runVaultProcess函数进行上锁保险箱并返回上锁状态标记。
 *  最后使用信号signalLockVault发送上锁状态标记。
 */
bool FileEncryptHandle::lockVault(QString unlockFileDir, bool isForced)
{
    fmInfo() << "Vault: Starting vault lock";
    d->activeState.insert(7, static_cast<int>(ErrorCode::kSuccess));
    int flg = d->lockVaultProcess(unlockFileDir, isForced);

    if (-1 == flg) {
        d->activeState.clear();
        fmCritical() << "Lock vault failed, progress crash!";
        return false;
    }

    if (d->activeState.value(7) != static_cast<int>(ErrorCode::kSuccess)) {
        emit signalLockVault(d->activeState.value(7));
        fmWarning() << "Lock vault failed! ";
        d->activeState.clear();
        return false;
    }

    d->curState = kEncrypted;
    emit signalLockVault(flg);
    fmInfo() << "Lock vault success!";
    d->activeState.clear();
    return true;
}

bool FileEncryptHandle::createDirIfNotExist(QString path)
{
    fmDebug() << "Vault: Checking vault directory existence";
    if (!QFile::exists(path)) {
        QDir().mkpath(path);
    } else {
        QDir dir(path);
        if (!dir.isEmpty()) {
            fmCritical() << "Vault: Create vault dir failed, dir is not empty!";
            return false;
        }
        fmDebug() << "Vault: The vault directory already exists and is empty";
    }
    return true;
}

VaultState FileEncryptHandle::state(const QString &encryptBaseDir, bool useCache) const
{
    if (encryptBaseDir.isEmpty()) {
        fmWarning() << "Vault: not set the base dir!";
        return kUnknow;
    }

    if (useCache && !(d->curState == kUnknow || d->curState == kEncrypted))
        return d->curState;

    const QString &cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        fmWarning() << "Vault: cryfs binary not found, vault not available";
        d->curState = kNotAvailable;
    } else {
        fmDebug() << "Vault: cryfs binary found at:" << cryfsBinary;

        QString lockBaseDir = encryptBaseDir;
        if (lockBaseDir.endsWith("/"))
            lockBaseDir += "cryfs.config";
        else
            lockBaseDir += "/cryfs.config";

        fmDebug() << "Vault: Checking config file:" << lockBaseDir;

        if (QFile::exists(lockBaseDir)) {
            fmDebug() << "Vault: Config file exists, checking mount status";

            QString realPath = QFileInfo(PathManager::vaultUnlockPath()).canonicalFilePath();
            if (realPath.isEmpty()) {
                fmDebug() << "Vault: Real path is empty, vault is encrypted";
                return kEncrypted;
            }

            QUrl mountDirUrl = QUrl::fromLocalFile(realPath);
            const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(mountDirUrl);
            fmDebug() << "Vault: Filesystem type:" << fsType;

            if (fsType == "fuse.cryfs") {
                d->curState = kUnlocked;
                fmDebug() << "Vault: Vault is unlocked (mounted)";
            } else {
                d->curState = kEncrypted;
                fmDebug() << "Vault: Vault is encrypted (not mounted)";
            }
        } else {
            d->curState = kNotExisted;
            fmDebug() << "Vault: Config file does not exist, vault not created";
        }
    }

    fmDebug() << "Vault: Final state determined:" << static_cast<int>(d->curState);
    return d->curState;
}

bool FileEncryptHandle::updateState(VaultState curState)
{
    fmDebug() << "Vault: Updating state from" << static_cast<int>(d->curState) << "to" << static_cast<int>(curState);

    if (curState == kNotExisted && d->curState != kEncrypted) {
        fmWarning() << "Vault: Invalid state transition - cannot set to NotExisted from current state";
        return false;
    }

    d->curState = curState;

    return true;
}

EncryptType FileEncryptHandle::encryptAlgoTypeOfGroupPolicy()
{
    return d->encryptAlgoTypeOfGroupPolicy();
}

/*!
 * \brief 进程执行错误时执行并发送signalReadError信号
 * \note
 *  根据获取的error值是否含有对应的标识来确定错误的具体原因
 */
void FileEncryptHandle::slotReadError()
{
    QString error = d->process->readAllStandardError().data();
    fmWarning() << "Vault: Process error output:" << error;

    if (d->activeState.contains(1)) {
        fmDebug() << "Vault: Processing error for vault creation (state 1)";
        if (error.contains("mountpoint is not empty")) {
            d->activeState[1] = static_cast<int>(ErrorCode::kMountpointNotEmpty);
            fmWarning() << "Vault: Creation failed - mountpoint is not empty";
        } else if (error.contains("Permission denied")) {
            d->activeState[1] = static_cast<int>(ErrorCode::kPermissionDenied);
            fmWarning() << "Vault: Creation failed - permission denied";
        }
    } else if (d->activeState.contains(3)) {
        fmDebug() << "Vault: Processing error for vault unlock (state 3)";
        if (error.contains("mountpoint is not empty")) {
            d->activeState[3] = static_cast<int>(ErrorCode::kMountpointNotEmpty);
            fmWarning() << "Vault: Unlock failed - mountpoint is not empty";
        } else if (error.contains("Permission denied")) {
            d->activeState[3] = static_cast<int>(ErrorCode::kPermissionDenied);
            fmWarning() << "Vault: Unlock failed - permission denied";
        }
    } else if (d->activeState.contains(7)) {
        fmDebug() << "Vault: Processing error for vault lock (state 7)";
        if (error.contains("Device or resource busy")) {
            d->activeState[7] = static_cast<int>(ErrorCode::kResourceBusy);
            fmWarning() << "Vault: Lock failed - device or resource busy";
        }
    }
    emit signalReadError(error);
}

/*!
 * \brief   进程执行过程中的输出信息，发送signalReadOutput信号
 */
void FileEncryptHandle::slotReadOutput()
{
    QString msg = d->process->readAllStandardOutput().data();
    fmDebug() << "Vault: Process output:" << msg;
    emit signalReadOutput(msg);
}

FileEncryptHandlerPrivate::FileEncryptHandlerPrivate(FileEncryptHandle *qq)
    : q(qq)
{
    fmDebug() << "Vault: Initializing FileEncryptHandlerPrivate";
    process = new QProcess;
    mutex = new QMutex;
    initEncryptType();
    fmDebug() << "Vault: FileEncryptHandlerPrivate initialization completed";
}

FileEncryptHandlerPrivate::~FileEncryptHandlerPrivate()
{
    fmDebug() << "Vault: Destroying FileEncryptHandlerPrivate";

    if (process) {
        delete process;
        process = nullptr;
    }

    if (mutex) {
        delete mutex;
        mutex = nullptr;
    }
}

/*!
 * \brief                        运行子进程执行创建保险箱或解锁保险箱,默认加密类型为aes-256-gcm,默认块大小32KB.
 * \param[in] lockBaseDir:       保险箱加密文件夹
 * \param[in] unlockFileDir:     保险箱解密文件夹
 * \param[in] passWord:          保险箱密码
 * \note
 *  检测cryfs应用程序是否存在，如果存在就使用QProcess创建子进程并结合参数运行cryfs，
 *  最后根据执行结果返回cryfs执行状态。此函数主要用于解锁保险箱
 * \return                       返回ErrorCode枚举值
 */
int FileEncryptHandlerPrivate::runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString DSecureString)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        fmCritical() << "Vault: cryfs binary not found";
        return static_cast<int>(ErrorCode::kCryfsNotExist);
    }
    fmDebug() << "Vault: Found cryfs binary at:" << cryfsBinary;

    QStringList arguments;
    CryfsVersionInfo version = versionString();
    if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
        arguments << QString("--allow-replaced-filesystem");
        fmDebug() << "Vault: Added --allow-replaced-filesystem argument";
    }
    arguments << lockBaseDir << unlockFileDir;
    fmDebug() << "Vault: Process arguments:" << arguments;

    setEnviroment(QPair<QString, QString>("CRYFS_FRONTEND", "noninteractive"));

    process->start(cryfsBinary, arguments);
    process->waitForStarted();
    process->write(DSecureString.toUtf8());
    process->waitForBytesWritten();
    process->closeWriteChannel();
    process->waitForFinished();
    process->terminate();

    if (process->exitStatus() == QProcess::NormalExit) {
        int exitCode = process->exitCode();
        fmDebug() << "Vault: Process exited normally with code:" << exitCode;
        return exitCode;
    } else {
        fmWarning() << "Vault: Process crashed or was terminated abnormally";
        return -1;
    }
}

/*!
 * \brief                       运行子进程执行创建保险箱或解锁保险箱
 * \param[in] lockBaseDir:      保险箱加密文件夹
 * \param[in] unlockFileDir:    保险箱解密文件夹
 * \param[in] passWord:         保险箱密码
 * \param[in] type:             加密类型
 * \param[in] blockSize:        解密文件块大小(大小影响加密文件的多少以及效率)
 * \note
 *  检测cryfs应用程序是否存在，如果存在就使用QProcess创建子进程并结合参数运行cryfs，
 *  最后根据执行结果返回cryfs执行状态。此函数主要用于创建保险箱
 * \return                      返回ErrorCode枚举值
 */
int FileEncryptHandlerPrivate::runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString DSecureString, EncryptType type, int blockSize)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        fmCritical() << "Vault: cryfs binary not found";
        return static_cast<int>(ErrorCode::kCryfsNotExist);
    }
    fmDebug() << "Vault: Found cryfs binary at:" << cryfsBinary;

    QStringList arguments;
    CryfsVersionInfo version = versionString();
    if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
        arguments << QString("--allow-replaced-filesystem");
        fmDebug() << "Vault: Added --allow-replaced-filesystem argument";
    }
    arguments << QString("--cipher") << encryptTypeMap.value(type) << QString("--blocksize") << QString::number(blockSize) << lockBaseDir << unlockFileDir;
    fmDebug() << "Vault: Process arguments:" << arguments;

    setEnviroment(QPair<QString, QString>("CRYFS_FRONTEND", "noninteractive"));

    process->start(cryfsBinary, arguments);
    process->waitForStarted();
    process->write(DSecureString.toUtf8());
    process->waitForBytesWritten();
    process->closeWriteChannel();
    process->waitForFinished();
    process->terminate();

    if (process->exitStatus() == QProcess::NormalExit) {
        int exitCode = process->exitCode();
        fmDebug() << "Vault: Process exited normally with code:" << exitCode;
        return exitCode;
    } else {
        fmWarning() << "Vault: Process crashed or was terminated abnormally";
        return -1;
    }
}

/*!
 * \brief                   加锁保险箱
 * \param[in] unlockFileDir 保险箱解密文件夹
 * \note
 *  检测cryfs应用程序是否存在，如果存在就使用QProcess创建子进程并结合参数运行cryfs，
 *  最后根据执行结果返回cryfs执行状态。此函数主要用于上锁保险箱
 * \return                  返回ErrorCode枚举值
 */
int FileEncryptHandlerPrivate::lockVaultProcess(QString unlockFileDir, bool isForced)
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
        fmDebug() << "Vault: Using forced unmount with -zu arguments";
    } else {
        arguments << "-u" << unlockFileDir;
        fmDebug() << "Vault: Using normal unmount with -u arguments";
    }

    if (fusermountBinary.isEmpty()) {
        fmCritical() << "Vault: fusermount binary not found";
        return static_cast<int>(ErrorCode::kFusermountNotExist);
    }
    fmDebug() << "Vault: Found fusermount binary at:" << fusermountBinary;
    fmDebug() << "Vault: Process arguments:" << arguments;

    fmDebug() << "Vault: Starting fusermount process";
    process->start(fusermountBinary, arguments);
    process->waitForStarted();
    process->waitForFinished();
    process->terminate();

    if (process->exitStatus() == QProcess::NormalExit) {
        int exitCode = process->exitCode();
        fmDebug() << "Vault: fusermount process exited normally with code:" << exitCode;
        return exitCode;
    } else {
        fmWarning() << "Vault: fusermount process crashed or was terminated abnormally";
        return -1;
    }
}

/*!
 * \brief 初始化加密类型选项
 */
void FileEncryptHandlerPrivate::initEncryptType()
{
    fmDebug() << "Vault: Initializing encryption type mappings";

    encryptTypeMap.insert(EncryptType::AES_256_GCM, "aes-256-gcm");
    encryptTypeMap.insert(EncryptType::AES_256_CFB, "aes-256-cfb");
    encryptTypeMap.insert(EncryptType::AES_128_GCM, "aes-128-gcm");
    encryptTypeMap.insert(EncryptType::AES_128_CFB, "aes-128-cfb");
    encryptTypeMap.insert(EncryptType::TWOFISH_256_GCM, "twofish-256-gcm");
    encryptTypeMap.insert(EncryptType::TWOFISH_256_CFB, "twofish-256-cfb");
    encryptTypeMap.insert(EncryptType::TWOFISH_128_GCM, "twofish-128-gcm");
    encryptTypeMap.insert(EncryptType::TWOFISH_128_CFB, "twofish-128-cfb");
    encryptTypeMap.insert(EncryptType::SERPENT_256_GCM, "serpent-256-gcm");
    encryptTypeMap.insert(EncryptType::SERPENT_256_CFB, "serpent-256-cfb");
    encryptTypeMap.insert(EncryptType::SERPENT_128_GCM, "serpent-128-gcm");
    encryptTypeMap.insert(EncryptType::SERPENT_128_CFB, "serpent-128-cfb");
    encryptTypeMap.insert(EncryptType::CAST_256_GCM, "cast-256-gcm");
    encryptTypeMap.insert(EncryptType::CAST_256_CFB, "cast-256-cfb");
    encryptTypeMap.insert(EncryptType::MARS_256_GCM, "mars-256-gcm");
    encryptTypeMap.insert(EncryptType::MARS_256_CFB, "mars-256-cfb");
    encryptTypeMap.insert(EncryptType::MARS_128_GCM, "mars-128-gcm");
    encryptTypeMap.insert(EncryptType::MARS_128_CFB, "mars-128-cfb");
    encryptTypeMap.insert(EncryptType::SM4_128_ECB, "sm4-128-ecb");
    encryptTypeMap.insert(EncryptType::SM4_128_CBC, "sm4-128-cbc");
    encryptTypeMap.insert(EncryptType::SM4_128_CFB, "sm4-128-cfb");
    encryptTypeMap.insert(EncryptType::SM4_128_OFB, "sm4-128-ofb");
    encryptTypeMap.insert(EncryptType::SM4_128_CTR, "sm4-128-ctr");
}

void FileEncryptHandlerPrivate::runVaultProcessAndGetOutput(const QStringList &arguments, QString &standardError, QString &standardOutput)
{
    fmDebug() << "Vault: Running vault process to get output";
    fmDebug() << "Vault: Arguments:" << arguments;

    const QString &cryfsProgram = QStandardPaths::findExecutable("cryfs");
    if (cryfsProgram.isEmpty()) {
        fmCritical() << "Vault: cryfs is not exist!";
        return;
    }
    fmDebug() << "Vault: Found cryfs program at:" << cryfsProgram;

    QProcess process;
    process.setEnvironment({ "CRYFS_FRONTEND=noninteractive", "CRYFS_NO_UPDATE_CHECK=true" });
    process.start(cryfsProgram, arguments);
    process.waitForStarted();
    process.waitForFinished();
    standardError = QString::fromLocal8Bit(process.readAllStandardError());
    standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
}

FileEncryptHandlerPrivate::CryfsVersionInfo FileEncryptHandlerPrivate::versionString()
{
    fmDebug() << "Vault: Getting cryfs version information";

    if (cryfsVersion.isVaild()) {
        fmDebug() << "Vault: Using cached version info";
        return cryfsVersion;
    }

    QString standardError { "" };
    QString standardOutput { "" };

    runVaultProcessAndGetOutput({ "--version" }, standardError, standardOutput);
    if (!standardOutput.isEmpty()) {
        fmDebug() << "Vault: Parsing version from output:" << standardOutput;

#if (QT_VERSION <= QT_VERSION_CHECK(5, 15, 0))
        QStringList &&datas = standardOutput.split('\n', QString::SkipEmptyParts);
#else
        QStringList &&datas = standardOutput.split('\n', Qt::SkipEmptyParts);
#endif
        if (!datas.isEmpty()) {
            const QString &data = datas.first();
            fmDebug() << "Vault: First line of version output:" << data;

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

QStringList FileEncryptHandlerPrivate::algoNameOfSupport()
{
    fmDebug() << "Vault: Getting supported algorithm names";

    QStringList result { "" };
    QString cryfsProgram = QStandardPaths::findExecutable("cryfs");
    if (cryfsProgram.isEmpty()) {
        fmCritical() << "Vault: cryfs is not exist!";
        return result;
    }
    fmDebug() << "Vault: Found cryfs program at:" << cryfsProgram;

    QProcess process;
    process.setEnvironment({ "CRYFS_FRONTEND=noninteractive", "CRYFS_NO_UPDATE_CHECK=true" });
    fmDebug() << "Vault: Starting process to get supported ciphers";

    process.start(cryfsProgram, { "--show-ciphers" });
    process.waitForStarted();
    process.waitForFinished();
    QString output = QString::fromLocal8Bit(process.readAllStandardError());
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    result = output.split('\n', QString::SkipEmptyParts);
#else
    result = output.split('\n', Qt::SkipEmptyParts);
#endif

    fmDebug() << "Vault: Found" << result.size() << "supported algorithms";
    return result;
}

bool FileEncryptHandlerPrivate::isSupportAlgoName(const QString &algoName)
{
    fmDebug() << "Vault: Checking if algorithm is supported:" << algoName;

    static QStringList algoNames = algoNameOfSupport();
    bool supported = algoNames.contains(algoName);

    fmDebug() << "Vault: Algorithm" << algoName << "supported:" << supported;
    return supported;
}

void FileEncryptHandlerPrivate::syncGroupPolicyAlgoName()
{
    fmDebug() << "Vault: Synchronizing group policy algorithm name";

    VaultConfig config;
    const QString &algoName = config.get(kConfigNodeName, kConfigKeyAlgoName, QVariant("NoExist")).toString();
    fmDebug() << "Vault: Retrieved algorithm name from config:" << algoName;

    if (algoName == "NoExist") {
        // 字段不存在，引入国密之前的保险箱，默认算法为aes-256-gcm
        const QString defaultAlgo = encryptTypeMap.value(EncryptType::AES_256_GCM);
        DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, defaultAlgo);
        fmInfo() << "Vault: Set default algorithm for legacy vault:" << defaultAlgo;
    } else {
        if (!algoName.isEmpty()) {
            DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, algoName);
            fmDebug() << "Vault: Updated group policy with algorithm:" << algoName;
        } else {
            fmWarning() << "Vault: Algorithm name is empty in config";
        }
    }
}

EncryptType FileEncryptHandlerPrivate::encryptAlgoTypeOfGroupPolicy()
{
    fmDebug() << "Vault: Getting encryption algorithm type from group policy";

    QString algoName { encryptTypeMap.value(EncryptType::SM4_128_ECB) };
    fmDebug() << "Vault: Default algorithm name:" << algoName;

    if (DConfigManager::instance()->contains(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName)) {
        algoName = DConfigManager::instance()->value(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, QVariant(kConfigKeyNotExist)).toString();
        fmDebug() << "Vault: Retrieved algorithm from group policy:" << algoName;

        if (algoName == kConfigKeyNotExist || algoName.isEmpty()) {
            algoName = encryptTypeMap.value(EncryptType::SM4_128_ECB);
            fmDebug() << "Vault: Using fallback algorithm:" << algoName;
        }
    } else {
        fmDebug() << "Vault: Group policy key not found, using default";
    }

    if (!isSupportAlgoName(algoName)) {
        fmWarning() << "Vault: Algorithm not supported:" << algoName;
        algoName = encryptTypeMap.value(EncryptType::SM4_128_ECB);
        if (!isSupportAlgoName(algoName)) {
            algoName = encryptTypeMap.value(EncryptType::AES_256_GCM);
            fmWarning() << "Vault: Fallback to AES-256-GCM:" << algoName;
        }
    }

    EncryptType type { EncryptType::AES_256_GCM };
    QMap<EncryptType, QString>::const_iterator i = encryptTypeMap.constBegin();
    while (i != encryptTypeMap.constEnd()) {
        if (i.value() == algoName) {
            type = i.key();
            break;
        }
        ++i;
    }

    return type;
}

void FileEncryptHandlerPrivate::setEnviroment(const QPair<QString, QString> &value)
{
    // Just append enviroment value, not replace.
    if (!process) {
        fmWarning() << "Vault: Process is null, cannot set environment";
        return;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(value.first, value.second);
    process->setProcessEnvironment(env);
    fmDebug() << "Vault: Environment variable set successfully";
}
