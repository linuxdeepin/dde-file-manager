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

#ifndef VAULTINTERFACE_H
#define VAULTINTERFACE_H

#include "dfm_filemanager_service_global.h"
#include "vault_defines.h"
#include "vaulterrorcode.h"

#include <dfm-framework/service/pluginservicecontext.h>

#include <QObject>
#include <QSharedPointer>

DSB_FM_BEGIN_NAMESPACE

class VaultHandle;
class VaultServicePrivate;
class VaultService final : public dpf::PluginService, dpf::AutoServiceRegister<VaultService>
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.VaultService";
    }

public:
    explicit VaultService(QObject *parent = nullptr);

    virtual ~VaultService() override;

    Q_ENUM(EncryptType)

    /*!
     * \brief                        创建保险箱,默认加密类型为aes-256-gcm,默认块大小32KB
     * \param[in] encryptBaseDir:    保险箱加密文件夹
     * \param[in] decryptFileDir:    保险箱解密文件夹
     * \param[in] DSecureString:     保险箱密码
     */
    void createVault(const QString &encryptBaseDir, const QString &decryptFileDir, const QString &DSecureString);

    /*!
     * \brief                       创建保险箱
     * \param[in] encryptBaseDir:   保险箱加密文件夹
     * \param[in] decryptFileDir:   保险箱解密文件夹
     * \param[in] DSecureString:    保险箱密码
     * \param[in] type:             加密类型
     * \param[in] blockSize:        解密文件块大小(大小影响加密文件的多少以及效率),大小建议4的整数倍
     */
    void createVault(const QString &encryptBaseDir, const QString &decryptFileDir, const QString &DSecureString, EncryptType type, int blockSize);

    /*!
     * \brief                         加锁保险箱
     * \param[in] decryptFileDir:     保险箱解密文件夹
     */
    void lockVault(const QString &decryptFileDir);

    /*!
     * \brief                        解锁保险箱
     * \param[in] encryptBaseDir:    保险箱加密文件夹
     * \param[in] decryptFileDir:    保险箱解密文件夹
     * \param[in] DSecureString:     保险箱密码
     */
    void unlockVault(const QString &encryptBaseDir, const QString &decryptFileDir, const QString &DSecureString);

    ServiceVaultState vaultState(const QString &encryptBaseDir, const QString &decryptFileDir);

signals:

    /*!
     * \brief                创建保险箱是否成功
     * \param[in] state:     创建保险箱状态
     */
    void signalCreateVaultState(int state);

    /*!
     * \brief                上锁是否成功
     * \param[in] state:     上锁状态
     */
    void signalLockVaultState(int state);

    /*!
     * \brief                解锁是否成功
     * \param[in] state:     解锁状态
     */
    void signalUnlockVaultState(int state);

    /*!
     * \brief                保险操作错误信息
     * \param[in] error:     错误信息
     */
    void signalError(QString error);

    /*!
     * \brief                保险箱操作输出信息
     * \param[in] msg:       输出信息
     */
    void signalOutputMessage(QString msg);

private:
    /*!
     * \brief  保险箱服务私有对象
     */
    QSharedPointer<VaultServicePrivate> vaultServicePrivate;
};
DSB_FM_END_NAMESPACE
#endif   // VAULTINTERFACE_H
