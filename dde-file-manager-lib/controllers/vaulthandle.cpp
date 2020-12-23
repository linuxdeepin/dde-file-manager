/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-05-08

 */


#include "vaulthandle.h"
#include "vaulterrorcode.h"
#include "dfmsettings.h"

#include <QStandardPaths>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMutex>
#include <QDateTime>
#include <unistd.h>

CryFsHandle::CryFsHandle(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    m_mutex = new QMutex;
    m_thread = new QThread;
    this->moveToThread(m_thread);
    connect(m_process , &QProcess::readyReadStandardError, this, &CryFsHandle::slotReadError);
    connect(m_process , &QProcess::readyReadStandardOutput, this, &CryFsHandle::slotReadOutput);
    m_thread->start();
}

void CryFsHandle::createVault(QString lockBaseDir, QString unlockFileDir, QString passWord)
{
    m_mutex->lock();
    m_activeState.insert(1, static_cast<int>(ErrorCode::Success));
    int flg = runVaultProcess(lockBaseDir, unlockFileDir,passWord);
    if(m_activeState.value(1) != static_cast<int>(ErrorCode::Success))
        emit signalCreateVault(m_activeState.value(1));
    else{
        emit signalCreateVault(flg);
        //! 记录保险箱创建时间
        DFM_NAMESPACE::DFMSettings setting(QString("vaultTimeConfig"));
        setting.setValue(QString("VaultTime"), QString("CreateTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    m_activeState.clear();
    m_mutex->unlock();
}

void CryFsHandle::unlockVault(QString lockBaseDir, QString unlockFileDir, QString passWord)
{
    m_mutex->lock();
    m_activeState.insert(3, static_cast<int>(ErrorCode::Success));
    qDebug() << "CryFsHandle::unlockVault:" << QThread::currentThread();
    int flg = runVaultProcess(lockBaseDir, unlockFileDir, passWord);
    if(m_activeState.value(3) != static_cast<int>(ErrorCode::Success))
        emit signalUnlockVault(m_activeState.value(3));
    else
        emit signalUnlockVault(flg);
    m_activeState.clear();
    m_mutex->unlock();
}

void CryFsHandle::lockVault(QString unlockFileDir)
{
    m_mutex->lock();
    m_activeState.insert(7, static_cast<int>(ErrorCode::Success));
    int flg = lockVaultProcess(unlockFileDir);
    if(m_activeState.value(7) != static_cast<int>(ErrorCode::Success))
        emit signalLockVault(m_activeState.value(7));
    else{
        emit signalLockVault(flg);
        //! 记录保险箱上锁时间
        DFM_NAMESPACE::DFMSettings setting(QString("vaultTimeConfig"));
        setting.setValue(QString("VaultTime"), QString("LockTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }
    m_activeState.clear();
    m_mutex->unlock();
}

int CryFsHandle::runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString passWord)
{
    QString cryfsBinary = QStandardPaths::findExecutable("cryfs");
    if (cryfsBinary.isEmpty()) return static_cast<int>(ErrorCode::CryfsNotExist);

    QStringList arguments;
    arguments << lockBaseDir << unlockFileDir;

    m_process->setEnvironment({"CRYFS_FRONTEND=noninteractive"});
    m_process->start(cryfsBinary, arguments);
    m_process->waitForStarted();
    m_process->write(passWord.toUtf8());
    m_process->waitForBytesWritten();
    m_process->closeWriteChannel();
    m_process->waitForFinished();
    m_process->terminate();

    if(m_process->exitStatus() == QProcess::NormalExit && m_process->exitCode() == 0)
        return static_cast<int>(ErrorCode::Success);
    else
        return m_process->exitCode();

}

int CryFsHandle::lockVaultProcess(QString unlockFileDir)
{
    QString fusermountBinary = QStandardPaths::findExecutable("fusermount");
    if (fusermountBinary.isEmpty()) return static_cast<int>(ErrorCode::FusermountNotExist);

    m_process->start(fusermountBinary, {"-zu", unlockFileDir});
    m_process->waitForStarted();
    m_process->waitForFinished();
    m_process->terminate();

    if(m_process->exitStatus() == QProcess::NormalExit && m_process->exitCode() == 0) {
        // 修复bug-52351
        // 保险箱上锁成功后,删除挂载目录
        if(rmdir(unlockFileDir.toStdString().c_str()) == -1) {
            qDebug() << "Vault Info: remove vault unlock dir failure";
        } else {
            qDebug() << "Vault Info: remove vault unlock dir success";
        }
        return static_cast<int>(ErrorCode::Success);
    }
    else
        return m_process->exitCode();
}

void CryFsHandle::slotReadError()
{
    QString error = m_process->readAllStandardError().data();
    if(m_activeState.contains(1))
    {
        if(error.contains("mountpoint is not empty"))
            m_activeState[1] = static_cast<int>(ErrorCode::MountpointNotEmpty);
        else if(error.contains("Permission denied"))
            m_activeState[1] = static_cast<int>(ErrorCode::PermissionDenied);
    }
    else if(m_activeState.contains(3))
    {
        if(error.contains("mountpoint is not empty"))
            m_activeState[3] = static_cast<int>(ErrorCode::MountpointNotEmpty);
        else if(error.contains("Permission denied"))
            m_activeState[3] = static_cast<int>(ErrorCode::PermissionDenied);
    }
    else if(m_activeState.contains(7))
    {
        if(error.contains("Device or resource busy"))
            m_activeState[7] = static_cast<int>(ErrorCode::ResourceBusy);
    }
    emit signalReadError(error);
}

void CryFsHandle::slotReadOutput()
{
    QString msg = m_process->readAllStandardOutput().data();
    emit signalReadOutput(msg);
}
