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
