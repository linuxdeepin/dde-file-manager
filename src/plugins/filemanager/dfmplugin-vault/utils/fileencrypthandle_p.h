// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHANDLE_P_H
#define VAULTHANDLE_P_H

#include "dfmplugin_vault_global.h"

#include <QMap>

class QMutex;
class QProcess;
class QThread;

namespace dfmplugin_vault {
class FileEncryptHandle;
class FileEncryptHandlerPrivate
{
    friend class FileEncryptHandle;
    Q_DISABLE_COPY(FileEncryptHandlerPrivate)

private:
    enum VersionIndex {
        kMajorIndex = 0,
        kMinorIndex,
        kHotFixIndex
    };
    struct CryfsVersionInfo
    {
        int majorVersion { -1 };
        int minorVersion { -1 };
        int hotfixVersion { -1 };

        CryfsVersionInfo(int major, int minor, int hotfix)
            : majorVersion(major), minorVersion(minor), hotfixVersion(hotfix)
        {
        }
        bool isOlderThan(const CryfsVersionInfo &version)
        {
            return (majorVersion < version.majorVersion)
                    || ((majorVersion == version.majorVersion) && ((minorVersion < version.minorVersion) || ((minorVersion == version.minorVersion) && (hotfixVersion < version.hotfixVersion))));
        }

        bool isVaild()
        {
            return (majorVersion > -1) && (minorVersion > -1) && (hotfixVersion > -1);
        }
    };

private:
    explicit FileEncryptHandlerPrivate(FileEncryptHandle *qq = nullptr);
    ~FileEncryptHandlerPrivate();

    int runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString DSecureString);
    int runVaultProcess(QString lockBaseDir, QString unlockFileDir, QString DSecureString, EncryptType type, int blockSize);
    int lockVaultProcess(QString unlockFileDir, bool isForced);
    void initEncryptType();
    void runVaultProcessAndGetOutput(const QStringList &arguments, QString &standardError, QString &standardOutput);
    CryfsVersionInfo versionString();

    QStringList algoNameOfSupport();
    bool isSupportAlgoName(const QString &algoName);
    void syncGroupPolicyAlgoName();
    EncryptType encryptAlgoTypeOfGroupPolicy();
    void setEnviroment(const QPair<QString, QString> &value);

private:
    QProcess *process { nullptr };
    QMutex *mutex { nullptr };
    QMap<int, int> activeState;
    QMap<EncryptType, QString> encryptTypeMap;
    FileEncryptHandle *q { nullptr };
    CryfsVersionInfo cryfsVersion { CryfsVersionInfo(-1, -1, -1) };
    VaultState curState { kUnknow };
};
}

#endif   // VAULTHANDLE_P_H
