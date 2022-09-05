// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>

class FileOperation;
class UserShareManager;
class UsbFormatter;
class CommandManager;
class DeviceInfoManager;
class TagManagerDaemon;
class AccessControlManager;
class VaultManager;
class RevocationManager;
class VaultBruteForcePrevention;

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    void initControllers();
    void initConnect();

signals:

public slots:

private:
    UserShareManager *m_userShareManager = nullptr;
    TagManagerDaemon *m_tagManagerDaemon = nullptr;
    AccessControlManager *m_accessController = nullptr;
    VaultManager *m_vaultManager = nullptr;
    RevocationManager *m_revocationManager = nullptr;
    VaultBruteForcePrevention *m_vaultForce = nullptr;
};

#endif // APPCONTROLLER_H
