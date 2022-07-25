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

#ifndef VAULTHANDLE_P_H
#define VAULTHANDLE_P_H

#include "dfmplugin_vault_global.h"

#include <QMap>

class QMutex;
class QProcess;
class QThread;

DPVAULT_BEGIN_NAMESPACE

class FileEncryptHandle;
class FileEncryptHandlerPrivate
{
    friend class FileEncryptHandle;
    Q_DISABLE_COPY(FileEncryptHandlerPrivate)

private:
    explicit FileEncryptHandlerPrivate(FileEncryptHandle *qq = nullptr);
    ~FileEncryptHandlerPrivate();

    int runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString DSecureString);
    int runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString DSecureString, EncryptType type, int blockSize);
    int lockVaultProcess(QString unlockFileDir);
    void initEncryptType();

private:
    QProcess *process { nullptr };
    QMutex *mutex { nullptr };
    QThread *thread { nullptr };
    QMap<int, int> activeState;
    QMap<EncryptType, QString> encryptTypeMap;
    FileEncryptHandle *q { nullptr };
};

DPVAULT_END_NAMESPACE

#endif   //VAULTHANDLE_P_H
