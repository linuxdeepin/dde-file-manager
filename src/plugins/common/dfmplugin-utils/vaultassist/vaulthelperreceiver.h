// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHELPERRECEIVER_H
#define VAULTHELPERRECEIVER_H

#include "dfmplugin_utils_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>

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
    bool handleSavePasswordToKeyring(const QString &password);
    bool handlemoveToTrash(const quint64 windowId, const QList<QUrl> sources,
                           const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

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
