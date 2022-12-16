/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef VAULTHELPERRECEIVER_H
#define VAULTHELPERRECEIVER_H

#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QDBusMessage>

namespace dfmplugin_utils {

inline constexpr int kArgumentsNum { 3 };

class VaultHelperReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultHelperReceiver)

public:
    enum VaultState {
        kNotExisted = 0,
        kEncrypted,
        kUnlocked,
        kUnderProcess,
        kBroken,
        kNotAvailable
    };

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

    explicit VaultHelperReceiver(QObject *parent = nullptr);
    void initEventConnect();

public slots:
    bool handleConnectLockScreenDBus();
    bool handleTransparentUnlockVault();
    QString handlePasswordFromKeyring();
    bool hanleSavePasswordToKeyring(const QString &password);

private slots:
    void responseLockScreenDBus(const QDBusMessage &msg);

private:
    bool transparentUnlockVault();
    VaultState state(const QString &encryptDir);
    int unlockVault(const QString &basedir, const QString &mountdir, const QString &passwd);
    CryfsVersionInfo versionString();
    void runVaultProcessAndGetOutput(const QStringList &arguments, QString &standardError, QString &standardOutput);
    void syncGroupPolicyAlgoName();
    int lockVault(QString unlockFileDir, bool isForced);

private:
    CryfsVersionInfo cryfsVersion { CryfsVersionInfo(-1, -1, -1) };
};

}
#endif // VAULTHELPERRECEIVER_H
