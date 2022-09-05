// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTDBUSRESPONSE_H
#define VAULTDBUSRESPONSE_H

#include <QObject>
#include <QDBusMessage>

class VaultDbusResponse : public QObject
{
    Q_OBJECT
public:
    enum VaultState {
        NotAvailable = 0,
        NotExisted,
        Encrypted,
        Unlocked
    };

    static VaultDbusResponse* instance();

    bool connectLockScreenDBus();
    bool transparentUnlockVault();

    static VaultState state();

private slots:
    void responseLockScreenDBus(const QDBusMessage &msg);

private:
    explicit VaultDbusResponse(QObject *parent = nullptr);
    int unlockVault(const QString &basedir, const QString &mountdir, const QString &passwd);
    void ChangeJson(const QString & path, const QString & gourpNamne, const QString & VauleName, const QString & vaule);
};

#endif // VAULTDBUSRESPONSE_H
