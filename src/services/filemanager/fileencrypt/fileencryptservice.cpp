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

#include "fileencryptservice.h"
#include "private/fileencryptservice_p.h"
#include "private/fileencrypthandle.h"
#include "fileencrypt_defines.h"

#include <QStorageInfo>
#include <QStandardPaths>
#include <QMetaType>
#include <QThread>
#include <QDebug>

DSB_FM_USE_NAMESPACE

FileEncryptService::FileEncryptService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<FileEncryptService>(),
      fileEncryptServicePrivate(new FileEncryptServicePrivate(this))
{
    connect(fileEncryptServicePrivate.data(), &FileEncryptServicePrivate::signalCreateVaultPrivate, this, &FileEncryptService::signalCreateVaultState, Qt::QueuedConnection);
    connect(fileEncryptServicePrivate.data(), &FileEncryptServicePrivate::signalLockVaultPrivate, this, &FileEncryptService::signalLockVaultState, Qt::QueuedConnection);
    connect(fileEncryptServicePrivate.data(), &FileEncryptServicePrivate::signalUnlockVaultPrivate, this, &FileEncryptService::signalUnlockVaultState, Qt::QueuedConnection);
    connect(fileEncryptServicePrivate.data(), &FileEncryptServicePrivate::signalReadErrorPrivate, this, &FileEncryptService::signalError, Qt::QueuedConnection);
    connect(fileEncryptServicePrivate.data(), &FileEncryptServicePrivate::signalReadOutputPrivate, this, &FileEncryptService::signalOutputMessage, Qt::QueuedConnection);
}

FileEncryptService::~FileEncryptService()
{
}

/*!
 * \brief                       创建保险箱,默认加密类型为aes-256-gcm,默认块大小32KB
 * \param[in] encryptBaseDir:   保险箱加密文件夹
 * \param[in] decryptFileDir:   保险箱解密文件夹
 * \param[in] DSecureString:    保险箱密码
 * \par example
 *      使用createVault函数创建后需要链接signalCreateVaultState信号接收创建的结果
 * \code
 *      //创建保险箱
 *      createVault(QString("/home/user/encrypt"), QString("home/user/decrypt"), QString("123456"));
 *      //接收保险箱创建结果
 *      connect(vaultService, &vaultService::signalCreateVaultState, this, &class::createVaultState);
 * \endcode
 */
void FileEncryptService::createVault(const QString &encryptBaseDir, const QString &decryptFileDir, const QString &DSecureString)
{
    emit fileEncryptServicePrivate->sigCreateVault(encryptBaseDir, decryptFileDir, DSecureString);
}

/*!
 * \brief                       创建保险箱
 * \param[in] encryptBaseDir:   保险箱加密文件夹
 * \param[in] decryptFileDir:   保险箱解密文件夹
 * \param[in] DSecureString:    保险箱密码
 * \param[in] type:             加密类型
 * \param[in] blockSize:        解密文件块大小(大小影响加密文件的多少以及效率),大小建议4的整数倍
 * \par example
 *      使用createVault函数创建后需要链接signalCreateVaultState信号接收创建的结果
 * \code
 *      //创建保险箱
 *      createVault(QString("/home/user/encrypt"), QString("home/user/decrypt"), QString("123456"), EncryptType::AES_256_GCM, 32768);
 *      //接收保险箱创建结果
 *      connect(vaultService, &vaultService::signalCreateVaultState, this, &class::createVaultState);
 * \endcode
 */
void FileEncryptService::createVault(const QString &encryptBaseDir, const QString &decryptFileDir, const QString &DSecureString, EncryptType type, int blockSize)
{
    emit fileEncryptServicePrivate->sigCreateVault(encryptBaseDir, decryptFileDir, DSecureString, type, blockSize);
}

/*!
 * \brief                        加锁保险箱
 * \param[in] decryptFileDir:    保险箱解密文件夹
 * \par example
 *      使用lockVault函数给保险箱上锁后需要链接signalLockVaultPrivate信号接收上锁的结果
 * \code
 *      //创建保险箱
 *      lockVault(QString("home/user/decrypt"));
 *      //接收保险箱创建结果
 *      connect(vaultService, &vaultService::signalLockVaultPrivate, this, &class::lockVault);
 * \endcode
 */
void FileEncryptService::lockVault(const QString &decryptFileDir)
{
    emit fileEncryptServicePrivate->sigLockVault(decryptFileDir);
}

/*!
 * \brief                       解锁保险箱
 * \param[in] encryptBaseDir:   保险箱加密文件夹
 * \param[in] decryptFileDir:   保险箱解密文件夹
 * \param[in] DSecureString:    保险箱密码
 * \par example
 *      使用unlockVault函数解锁后需要链接signalUnlockVaultPrivate信号接收解锁的结果
 * \code
 *      //创建保险箱
 *      unlockVault(QString("/home/user/encrypt"), QString("home/user/decrypt"), QString("123456"));
 *      //接收保险箱创建结果
 *      connect(vaultService, &vaultService::signalUnlockVaultPrivate, this, &class::unlockVault);
 * \endcode
 */
void FileEncryptService::unlockVault(const QString &encryptBaseDir, const QString &decryptFileDir, const QString &DSecureString)
{
    emit fileEncryptServicePrivate->sigUnlockVault(encryptBaseDir, decryptFileDir, DSecureString);
}

ServiceVaultState FileEncryptService::vaultState(const QString &encryptBaseDir, const QString &decryptFileDir)
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

FileEncryptServicePrivate::FileEncryptServicePrivate(QObject *parent)
    : QObject(parent),
      vaultHandle(new FileEncryptHandle())
{

    connect(this, &FileEncryptServicePrivate::sigCreateVault, vaultHandle.data(), &FileEncryptHandle::createVault, Qt::QueuedConnection);
    connect(this, &FileEncryptServicePrivate::sigUnlockVault, vaultHandle.data(), &FileEncryptHandle::unlockVault, Qt::QueuedConnection);
    connect(this, &FileEncryptServicePrivate::sigLockVault, vaultHandle.data(), &FileEncryptHandle::lockVault, Qt::QueuedConnection);

    connect(vaultHandle.data(), &FileEncryptHandle::signalCreateVault, this, &FileEncryptServicePrivate::signalCreateVaultPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &FileEncryptHandle::signalLockVault, this, &FileEncryptServicePrivate::signalLockVaultPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &FileEncryptHandle::signalUnlockVault, this, &FileEncryptServicePrivate::signalUnlockVaultPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &FileEncryptHandle::signalReadError, this, &FileEncryptServicePrivate::signalReadErrorPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &FileEncryptHandle::signalReadOutput, this, &FileEncryptServicePrivate::signalReadOutputPrivate, Qt::QueuedConnection);
}

FileEncryptServicePrivate::~FileEncryptServicePrivate()
{
}
