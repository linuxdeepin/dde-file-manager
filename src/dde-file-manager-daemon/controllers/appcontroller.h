/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
