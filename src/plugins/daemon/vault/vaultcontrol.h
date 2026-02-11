// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTCONTROL_H
#define VAULTCONTROL_H

#include "daemonplugin_vaultdaemon_global.h"

#include <QObject>
#include <QDBusMessage>

DAEMONPVAULT_BEGIN_NAMESPACE
class VaultControl : public QObject
{
    Q_OBJECT
public:
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

    static VaultControl* instance();
    void connectLockScreenDBus();
    bool transparentUnlockVault();
    void MonitorNetworkStatus();

private Q_SLOTS:
    void responseLockScreenDBus(const QDBusMessage &msg);
    void responseNetworkStateChaneDBus(int st);

Q_SIGNALS:
    void changedVaultState(const QVariantMap &map);

private:
    explicit VaultControl(QObject *parent = nullptr);
    int lockVault(const QString &unlockFileDir, bool isForced);
    CryfsVersionInfo versionString();
    void runVaultProcessAndGetOutput(const QStringList &arguments, QString &standardError, QString &standardOutput);
    VaultState state(const QString &encryptDir);
    void syncGroupPolicyAlgoName();
    QString passwordFromKeyring();
    int unlockVault(const QString &basedir, const QString &mountdir, const QString &passwd);

private:
    CryfsVersionInfo cryfsVersion { CryfsVersionInfo(-1, -1, -1) };
};
DAEMONPVAULT_END_NAMESPACE

#endif // VAULTCONTROL_H
