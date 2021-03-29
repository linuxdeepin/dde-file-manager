/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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


#ifndef CRYFSHANDLE_H
#define CRYFSHANDLE_H

#include <QObject>
#include <QMap>

class QMutex;
class QProcess;
class QThread;
class CryFsHandle : public QObject
{
    Q_OBJECT
public:
    explicit CryFsHandle(QObject *parent = nullptr);
    ~CryFsHandle();

    /**
     * @brief createVault       创建保险箱
     * @param lockBaseDir       保险箱加密文件夹
     * @param unlockFileDir     保险箱解密文件夹
     * @param passWord          保险箱密码
     */
    void createVault(QString lockBaseDir, QString unlockFileDir, QString passWord);

    /**
     * @brief unlockVault       解锁保险箱
     * @param lockBaseDir       保险箱加密文件夹
     * @param unlockFileDir     保险箱解密文件夹
     * @param passWord          保险箱密码
     */
    void unlockVault(QString lockBaseDir, QString unlockFileDir, QString passWord);

    /**
     * @brief lockedStrongBox   加锁保险箱
     * @param unlockFileDir     保险箱解密文件夹
     */
    void lockVault(QString unlockFileDir);

private:
    /**
     * @brief runVaultProcess   运行子进程执行创建保险箱或解锁保险箱
     * @param lockBaseDir       保险箱加密文件夹
     * @param unlockFileDir     保险箱解密文件夹
     * @param passWord          保险箱密码
     * @return                  返回ErrorCode枚举值
     */
    int runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString passWord);

    /**
     * @brief vaultLockProcess  加锁保险箱
     * @param unlockFileDir     保险箱解密文件夹
     * @return                  返回ErrorCode枚举值
     */
    int lockVaultProcess(QString unlockFileDir);

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

public slots:
    /**
     * @brief slotReadError     进程执行错误时执行并发送signalReadError信号
     */
    void slotReadError();

    /**
     * @brief slotReadOutput    进程执行过程中的输出信息，发送signalReadOutput信号
     */
    void slotReadOutput();

private:
    QProcess    * m_process;
    QThread     * m_thread;

    QMutex      * m_mutex;
    QMap<int, int> m_activeState;
};



#endif // CRYFSHANDLE_H
