/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "fileencrypthandle.h"
#include "fileencrypthandle_p.h"
#include "encryption/vaultconfig.h"

#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

#include <QDirIterator>
#include <QStandardPaths>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QDateTime>
#include <QStorageInfo>

#include <unistd.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FileEncryptHandle::FileEncryptHandle(QObject *parent)
    : QObject(parent), d(new FileEncryptHandlerPrivate(this))
{
    this->moveToThread(d->thread);
    connect(d->process, &QProcess::readyReadStandardError, this, &FileEncryptHandle::slotReadError);
    connect(d->process, &QProcess::readyReadStandardOutput, this, &FileEncryptHandle::slotReadOutput);
    d->thread->start();
}

FileEncryptHandle::~FileEncryptHandle()
{
    disconnect(d->process, &QProcess::readyReadStandardError, this, &FileEncryptHandle::slotReadError);
    disconnect(d->process, &QProcess::readyReadStandardOutput, this, &FileEncryptHandle::slotReadOutput);

    delete d;
    d = nullptr;
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
void FileEncryptHandle::createVault(QString lockBaseDir, QString unlockFileDir, QString passWord, EncryptType type, int blockSize)
{
    d->mutex->lock();
    d->activeState.insert(1, static_cast<int>(ErrorCode::kSuccess));
    createDirIfNotExist(lockBaseDir);
    createDirIfNotExist(unlockFileDir);

    const QString &algoName = d->encryptTypeMap.value(type);
    DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, algoName);
    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyAlgoName, QVariant(algoName));

    int flg = d->runVaultProcess(lockBaseDir, unlockFileDir, passWord, type, blockSize);
    if (d->activeState.value(1) != static_cast<int>(ErrorCode::kSuccess)) {
        emit signalCreateVault(d->activeState.value(1));
        qInfo() << "create fail";
    } else {
        emit signalCreateVault(flg);
        qInfo() << "create success " << flg;
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
void FileEncryptHandle::unlockVault(QString lockBaseDir, QString unlockFileDir, QString DSecureString)
{
    d->mutex->lock();
    d->activeState.insert(3, static_cast<int>(ErrorCode::kSuccess));
    qDebug() << "VaultHandle::unlockVault:" << QThread::currentThread();

    d->syncGroupPolicyAlgoName();

    createDirIfNotExist(unlockFileDir);
    int flg = d->runVaultProcess(lockBaseDir, unlockFileDir, DSecureString);
    if (d->activeState.value(3) != static_cast<int>(ErrorCode::kSuccess)) {
        emit signalUnlockVault(d->activeState.value(3));
        qInfo() << "Decrypt fail";
    } else {
        emit signalUnlockVault(flg);
        qInfo() << "Decrypt success " << flg;
    }
    d->activeState.clear();
    d->mutex->unlock();
}

/*!
 * \brief                        加锁保险箱
 * \param[in] unlockFileDir:     保险箱解密文件夹
 * \note
 *  使用map容器activeState的key为7来记录上锁状态标记，
 *  调用runVaultProcess函数进行上锁保险箱并返回上锁状态标记。
 *  最后使用信号signalLockVault发送上锁状态标记。
 */
void FileEncryptHandle::lockVault(QString unlockFileDir)
{
    d->mutex->lock();
    d->activeState.insert(7, static_cast<int>(ErrorCode::kSuccess));
    int flg = d->lockVaultProcess(unlockFileDir);
    if (d->activeState.value(7) != static_cast<int>(ErrorCode::kSuccess)) {
        emit signalLockVault(d->activeState.value(7));
        qInfo() << "encrypt fial";
    } else {
        emit signalLockVault(flg);
        qInfo() << "encrypt success " << flg;
    }
    d->activeState.clear();
    d->mutex->unlock();
}

void FileEncryptHandle::createDirIfNotExist(QString path)
{
    if (!QFile::exists(path)) {
        QDir().mkpath(path);
    } else {   // 修复bug-52351 创建保险箱前，如果文件夹存在，则清空
        QDir dir(path);
        if (!dir.isEmpty()) {
            QDirIterator dirsIterator(path, QDir::AllEntries | QDir::NoDotAndDotDot);
            while (dirsIterator.hasNext()) {
                if (!dir.remove(dirsIterator.next())) {
                    QDir(dirsIterator.filePath()).removeRecursively();
                }
            }
        }
    }
}

VaultState FileEncryptHandle::state(const QString &encryptBaseDir, const QString &decryptFileDir) const
{
    const QString &cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) {
        // 记录保险箱状态
        return kNotAvailable;
    }
    QString lockBaseDir = encryptBaseDir;

    if (lockBaseDir.endsWith("/"))
        lockBaseDir += "cryfs.config";
    else
        lockBaseDir += "/cryfs.config";

    if (QFile::exists(lockBaseDir)) {
        QStorageInfo info(decryptFileDir);
        const QString &temp = info.fileSystemType();
        if (info.isValid() && temp == "fuse.cryfs") {
            return kUnlocked;
        }
        return kEncrypted;
    } else {
        return kNotExisted;
    }
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
    qInfo() << error;
    if (d->activeState.contains(1)) {
        if (error.contains("mountpoint is not empty"))
            d->activeState[1] = static_cast<int>(ErrorCode::kMountpointNotEmpty);
        else if (error.contains("Permission denied"))
            d->activeState[1] = static_cast<int>(ErrorCode::kPermissionDenied);
    } else if (d->activeState.contains(3)) {
        if (error.contains("mountpoint is not empty"))
            d->activeState[3] = static_cast<int>(ErrorCode::kMountpointNotEmpty);
        else if (error.contains("Permission denied"))
            d->activeState[3] = static_cast<int>(ErrorCode::kPermissionDenied);
    } else if (d->activeState.contains(7)) {
        if (error.contains("Device or resource busy"))
            d->activeState[7] = static_cast<int>(ErrorCode::kResourceBusy);
    }
    emit signalReadError(error);
}

/*!
 * \brief   进程执行过程中的输出信息，发送signalReadOutput信号
 */
void FileEncryptHandle::slotReadOutput()
{
    QString msg = d->process->readAllStandardOutput().data();
    emit signalReadOutput(msg);
}

FileEncryptHandlerPrivate::FileEncryptHandlerPrivate(FileEncryptHandle *qq)
    : q(qq)
{
    process = new QProcess;
    mutex = new QMutex;
    thread = new QThread();
    initEncryptType();
}

FileEncryptHandlerPrivate::~FileEncryptHandlerPrivate()
{
    if (process) {
        delete process;
        process = nullptr;
    }

    if (mutex) {
        delete mutex;
        mutex = nullptr;
    }

    if (thread) {
        thread->quit();
        thread->deleteLater();
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
    if (cryfsBinary.isEmpty()) return static_cast<int>(ErrorCode::kCryfsNotExist);

    QStringList arguments;
    CryfsVersionInfo version = versionString();
    if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
        arguments << QString("--allow-replaced-filesystem");
    }
    arguments << lockBaseDir << unlockFileDir;

    process->setEnvironment({ "CRYFS_FRONTEND=noninteractive" });
    process->start(cryfsBinary, arguments);
    process->waitForStarted();
    process->write(DSecureString.toUtf8());
    process->waitForBytesWritten();
    process->closeWriteChannel();
    process->waitForFinished();
    process->terminate();

    if (process->exitStatus() == QProcess::NormalExit)
        return process->exitCode();
    else
        return -1;
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
    if (cryfsBinary.isEmpty()) return static_cast<int>(ErrorCode::kCryfsNotExist);

    QStringList arguments;
    CryfsVersionInfo version = versionString();
    if (version.isVaild() && !version.isOlderThan(CryfsVersionInfo(0, 10, 0))) {
        arguments << QString("--allow-replaced-filesystem");
    }
    arguments << QString("--cipher") << encryptTypeMap.value(type) << QString("--blocksize") << QString::number(blockSize) << lockBaseDir << unlockFileDir;

    qInfo() << arguments;

    process->setEnvironment({ "CRYFS_FRONTEND=noninteractive" });
    process->start(cryfsBinary, arguments);
    process->waitForStarted();
    process->write(DSecureString.toUtf8());
    process->waitForBytesWritten();
    process->closeWriteChannel();
    process->waitForFinished();
    process->terminate();

    if (process->exitStatus() == QProcess::NormalExit)
        return process->exitCode();
    else
        return -1;
}

/*!
 * \brief                   加锁保险箱
 * \param[in] unlockFileDir 保险箱解密文件夹
 * \note
 *  检测cryfs应用程序是否存在，如果存在就使用QProcess创建子进程并结合参数运行cryfs，
 *  最后根据执行结果返回cryfs执行状态。此函数主要用于上锁保险箱
 * \return                  返回ErrorCode枚举值
 */
int FileEncryptHandlerPrivate::lockVaultProcess(QString unlockFileDir)
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
        arguments << "-u" << unlockFileDir;
    }
    if (fusermountBinary.isEmpty()) return static_cast<int>(ErrorCode::kFusermountNotExist);

    process->start(fusermountBinary, arguments);
    process->waitForStarted();
    process->waitForFinished();
    process->terminate();

    if (process->exitStatus() == QProcess::NormalExit)
        return process->exitCode();
    else
        return -1;
}

/*!
 * \brief 初始化加密类型选项
 */
void FileEncryptHandlerPrivate::initEncryptType()
{
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

FileEncryptHandlerPrivate::CryfsVersionInfo FileEncryptHandlerPrivate::versionString()
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

QStringList FileEncryptHandlerPrivate::algoNameOfSupport()
{
    QStringList result { "" };
    QString cryfsProgram = QStandardPaths::findExecutable("cryfs");
    if (cryfsProgram.isEmpty()) {
        qWarning() << "cryfs is not exist!";
        return result;
    }

    QProcess process;
    process.setEnvironment({ "CRYFS_FRONTEND=noninteractive", "CRYFS_NO_UPDATE_CHECK=true" });
    process.start(cryfsProgram, { "--show-ciphers" });
    process.waitForStarted();
    process.waitForFinished();
    QString output = QString::fromLocal8Bit(process.readAllStandardError());
    result = output.split('\n', QString::SkipEmptyParts);

    return result;
}

bool FileEncryptHandlerPrivate::isSupportAlgoName(const QString &algoName)
{
    static QStringList algoNames = algoNameOfSupport();
    if (algoNames.contains(algoName))
        return true;
    return false;
}

void FileEncryptHandlerPrivate::syncGroupPolicyAlgoName()
{
    VaultConfig config;
    const QString &algoName = config.get(kConfigNodeName, kConfigKeyAlgoName, QVariant("NoExist")).toString();
    if (algoName == "NoExist") {
        // 字段不存在，引入国密之前的保险箱，默认算法为aes-256-gcm
        DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, encryptTypeMap.value(EncryptType::AES_256_GCM));
    } else {
        if (!algoName.isEmpty())
            DConfigManager::instance()->setValue(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, algoName);
    }
}

EncryptType FileEncryptHandlerPrivate::encryptAlgoTypeOfGroupPolicy()
{
    QString algoName { encryptTypeMap.value(EncryptType::SM4_128_ECB) };
    if (DConfigManager::instance()->contains(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName)) {
        algoName = DConfigManager::instance()->value(kDefaultCfgPath, kGroupPolicyKeyVaultAlgoName, QVariant(kConfigKeyNotExist)).toString();
        if (algoName == kConfigKeyNotExist || algoName.isEmpty()) {
            algoName = encryptTypeMap.value(EncryptType::SM4_128_ECB);
        }
    }

    if (!isSupportAlgoName(algoName)) {
        algoName = encryptTypeMap.value(EncryptType::SM4_128_ECB);
        if (!isSupportAlgoName(algoName))
            algoName = encryptTypeMap.value(EncryptType::AES_256_GCM);
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
