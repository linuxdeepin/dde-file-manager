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

#ifndef VAULTHANDLE_H
#define VAULTHANDLE_H

#include "dfm_filemanager_service_global.h"
#include "vault/vaulterrorcode.h"

#include <QObject>

DSB_FM_BEGIN_NAMESPACE

class VaultService;
class VaultHandlePrivate;
class VaultHandle : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultHandle)
private:
    friend class VaultServicePrivate;
    explicit VaultHandle(QObject *parent = nullptr);

public:
    virtual ~VaultHandle() override;

private:
    void createVault(QString lockBaseDir, QString unlockFileDir, QString DSecureString, EncryptType type = EncryptType::AES_256_GCM, int blockSize = 32768);

    void unlockVault(QString lockBaseDir, QString unlockFileDir, QString DSecureString);

    void lockVault(QString unlockFileDir);

signals:
    /*!
     * \brief                错误输出
     * \param[in] error:     错误信息
     */
    void signalReadError(QString error);

    /*!
     * \brief                标准输出
     * \param[in] msg:       输出信息
     */
    void signalReadOutput(QString msg);

    /*!
     * \brief                创建保险箱是否成功的信号
     * \param[in] state:     返回ErrorCode枚举值
     */
    void signalCreateVault(int state);

    /*!
     * \brief                解锁保险箱是否成功的信号
     * \param[in] state:     返回ErrorCode枚举值
     */
    void signalUnlockVault(int state);

    /*!
     * \brief                加锁保险箱是否成功的信号
     * \param[in] state      返回ErrorCode枚举值
     */
    void signalLockVault(int state);

private slots:
    void slotReadError();

    void slotReadOutput();

private:
    VaultHandlePrivate *vaultHandlePrivatePtr = nullptr;
};
DSB_FM_END_NAMESPACE
#endif   // VAULTHANDLE_H
