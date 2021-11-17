/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
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

#include "vaultservice.h"
#include "private/vaultservice_p.h"
#include "private/vaulthandle.h"

#include <QMetaType>
#include <QThread>
#include <QDebug>

DSB_FM_USE_NAMESPACE

VaultService::VaultService(QObject *parent)
    : dpf::PluginService(parent),
      dpf::AutoServiceRegister<VaultService>(),
      vaultServicePrivate(new VaultServicePrivate(this))
{
    connect(vaultServicePrivate.data(), &VaultServicePrivate::signalCreateVaultPrivate, this, &VaultService::signalCreateVaultState, Qt::QueuedConnection);
    connect(vaultServicePrivate.data(), &VaultServicePrivate::signalLockVaultPrivate, this, &VaultService::signalLockVaultState, Qt::QueuedConnection);
    connect(vaultServicePrivate.data(), &VaultServicePrivate::signalUnlockVaultPrivate, this, &VaultService::signalUnlockVaultState, Qt::QueuedConnection);
    connect(vaultServicePrivate.data(), &VaultServicePrivate::signalReadErrorPrivate, this, &VaultService::signalError, Qt::QueuedConnection);
    connect(vaultServicePrivate.data(), &VaultServicePrivate::signalReadOutputPrivate, this, &VaultService::signalOutputMessage, Qt::QueuedConnection);
}

VaultService::~VaultService()
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
void VaultService::createVault(QString encryptBaseDir, QString decryptFileDir, QString DSecureString)
{
    emit vaultServicePrivate->sigCreateVault(encryptBaseDir, decryptFileDir, DSecureString);
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
void VaultService::createVault(QString encryptBaseDir, QString decryptFileDir, QString DSecureString, EncryptType type, int blockSize)
{
    emit vaultServicePrivate->sigCreateVault(encryptBaseDir, decryptFileDir, DSecureString, type, blockSize);
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
void VaultService::lockVault(QString decryptFileDir)
{
    emit vaultServicePrivate->sigLockVault(decryptFileDir);
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
void VaultService::unlockVault(QString encryptBaseDir, QString decryptFileDir, QString DSecureString)
{
    emit vaultServicePrivate->sigUnlockVault(encryptBaseDir, decryptFileDir, DSecureString);
}

VaultServicePrivate::VaultServicePrivate(QObject *parent)
    : QObject(parent),
      vaultHandle(new VaultHandle())
{
    qRegisterMetaType<EncryptType>("encryptType");

    connect(this, &VaultServicePrivate::sigCreateVault, vaultHandle.data(), &VaultHandle::createVault, Qt::QueuedConnection);
    connect(this, &VaultServicePrivate::sigUnlockVault, vaultHandle.data(), &VaultHandle::unlockVault, Qt::QueuedConnection);
    connect(this, &VaultServicePrivate::sigLockVault, vaultHandle.data(), &VaultHandle::lockVault, Qt::QueuedConnection);

    connect(vaultHandle.data(), &VaultHandle::signalCreateVault, this, &VaultServicePrivate::signalCreateVaultPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &VaultHandle::signalLockVault, this, &VaultServicePrivate::signalLockVaultPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &VaultHandle::signalUnlockVault, this, &VaultServicePrivate::signalUnlockVaultPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &VaultHandle::signalReadError, this, &VaultServicePrivate::signalReadErrorPrivate, Qt::QueuedConnection);
    connect(vaultHandle.data(), &VaultHandle::signalReadOutput, this, &VaultServicePrivate::signalReadOutputPrivate, Qt::QueuedConnection);
}

VaultServicePrivate::~VaultServicePrivate()
{
}
