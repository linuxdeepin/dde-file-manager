/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#pragma once

#include "dabstractfilecontroller.h"

#include <DSecureString>

DCORE_USE_NAMESPACE

class VaultControllerPrivate;
class VaultInterface;
class VaultController : public DAbstractFileController
{
    Q_OBJECT
public:
    enum VaultState {
        NotExisted,
        Encrypted,
        Unlocked,
        UnderProcess,
        Broken,
        NotAvailable
    };

    explicit VaultController(QObject *parent = nullptr);
public:

    static VaultController * getVaultController();

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    const DDirIteratorPointer createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const Q_DECL_OVERRIDE;
    DUrlList moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const override;
    bool writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const override;
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;
    bool deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const override;

    static DUrl makeVaultUrl(QString path = "", QString host = "files");
    static DUrl localUrlToVault(const DUrl &vaultUrl);
    static DUrl localToVault(QString localPath);
    static QString vaultToLocal(const DUrl &vaultUrl);
    static DUrl vaultToLocalUrl(const DUrl &vaultUrl);
    static DUrlList vaultToLocalUrls(DUrlList vaultUrls);

    /**
    * @brief checkAuthentication    权限校验
    */
    static bool checkAuthentication();

    /**
     * @brief state         获取当前保险箱状态
     * @param lockBaseDir   保险箱加密文件夹
     * @return              返回VaultState枚举值
     */
    VaultState state(QString lockBaseDir = "");

public slots:

    /**
     * @brief createVault       创建保险箱
     * @param lockBaseDir       保险箱加密文件夹 默认值内部自动创建
     * @param unlockFileDir     保险箱解密文件夹 默认值内部自动创建
     * @param passWord          保险箱密码

     */
    void createVault(const DSecureString &password, QString lockBaseDir = "", QString unlockFileDir = "");

    /**
     * @brief unlockVault       解锁保险箱
     * @param lockBaseDir       保险箱加密文件夹 默认值内部自动创建
     * @param unlockFileDir     保险箱解密文件夹 默认值内部自动创建
     * @param passWord          保险箱密码
     */
    void unlockVault(const DSecureString &password, QString lockBaseDir = "", QString unlockFileDir = "");

    /**
     * @brief lockVault         加锁保险箱
     * @param lockBaseDir       保险箱加密文件夹 默认值内部自动创建
     * @param unlockFileDir     保险箱解密文件夹 默认值内部自动创建
     */
    void lockVault(QString lockBaseDir = "", QString unlockFileDir = "");

    /**
     * @brief makeVaultLocalPath    创建本地路径
     * @param path                  子目录(文件或文件路径)
     * @param base                  父目录
     * @return                      返回新路径
     */
    static QString makeVaultLocalPath(QString path = "", QString base = "");

    /**
     * @brief vaultLockPath   返回默认保险箱加密文件夹路径，如路径是外部传入暂时无法获取
     * @return                默认保险箱加密文件夹路径
     */
    static QString vaultLockPath();

    /**
     * @brief vaultLockPath   返回默认保险箱解密文件夹路径，如路径是外部传入暂时无法获取
     * @return                默认保险箱解密文件夹路径
     */
    static QString vaultUnlockPath();

signals:
    /**
     * @brief readError 错误输出
     * @param error     错误信息
     */
    void signalReadError(QString error);

    /**
     * @brief signalReadOutput  标准输出
     * @param msg               输出信息
     */
    void signalReadOutput(QString msg);

    /**
     * @brief signalCreateVault 创建保险箱是否成功的信号
     * @param state             返回ErrorCode枚举值
     */
    void signalCreateVault(int state);

    /**
     * @brief singalUnlockVault 解锁保险箱是否成功的信号
     * @param state             返回ErrorCode枚举值
     */
    void signalUnlockVault(int state);

    /**
     * @brief signalLockVault   加锁保险箱是否成功的信号
     * @param state             返回ErrorCode枚举值
     */
    void signalLockVault(int state);

signals:
    /**
    * @brief 下列信号为本类内部使用，请勿外用
    */

    /**
     * @brief sigCreateVault    创建保险箱信号
     * @param lockBaseDir       保险箱加密文件夹
     * @param unlockFileDir     保险箱解密文件夹
     * @param passWord          保险箱密码
     */
    void sigCreateVault(QString lockBaseDir, QString unlockFileDir, QString passWord);

    /**
     * @brief sigUnlockVault    解锁保险箱信号
     * @param lockBaseDir       保险箱加密文件夹
     * @param unlockFileDir     保险箱解密文件夹
     * @param passWord          保险箱密码
     */
    void sigUnlockVault(QString lockBaseDir, QString unlockFileDir, QString passWord);

    /**
     * @brief sigLockVault      加锁保险箱信号
     * @param unlockFileDir     保险箱解密文件夹
     */
    void sigLockVault(QString unlockFileDir);

private:
    static bool runCmd(const QString &cmd, QString &standOutput);

private:
    VaultControllerPrivate * d_ptr;

    static VaultController * cryfs;

    Q_DECLARE_PRIVATE(VaultController)
};
