/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#ifndef VAULTHELPER_H
#define VAULTHELPER_H

#include "dfmplugin_vault_global.h"
#include "utils/vaultglobaldefine.h"
#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/vault/vaultservice.h"

#include <QObject>
#include <QIcon>
#include <QMenu>

DPVAULT_BEGIN_NAMESPACE
class VaultHelper final : public QObject
{
    Q_OBJECT
public:
    inline static QString scheme()
    {
        return "dfmvault";
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("drive-harddisk-encrypted-symbolic");
    }

    static QUrl rootUrl();

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

    static void siderItemClicked(quint64 windowId, const QUrl &url);

    /*!
    * @brief isVaultEnabled
    * @return true vault is available, vice versa.
    */
    static bool isVaultEnabled();

    static VaultState state(QString lockBaseDir);

    static QString makeVaultLocalPath(QString path = "", QString base = "");

    static QList<DSB_FM_NAMESPACE::TitleBar::CrumbData> seprateUrl(const QUrl &url);

    static DSB_FM_NAMESPACE::VaultService *vaultServiceInstance();

    /**
     * @brief getVaultVersion   获取当前保险箱版本是否是1050及以上版本
     * @return  true大于等于1050,false小于1050
     */
    static bool getVaultVersion();

    static QString vaultLockPath();

    static QString vaultUnlockPath();

    static QMenu *createMenu();

    static QWidget *createVaultPropertyDialog(const QUrl &url);

public:
    // 定义静态变量，记录当前保险箱是否处于模态弹窗状态
    static bool isModel;

public slots:
    static void slotlockVault(int state);

    static void createVault(QString &password);

    static void unlockVault(QString &password);

    static void lockVault();

    static void defaultCdAction(const QUrl &url);

    static void openNewWindow(const QUrl &url);

    void creatVaultDialog();

    void unlockVaultDialog();

    void removeVaultDialog();

    void openWindow();

    void openWidWindow(quint64 winID, const QUrl &url);

    void newOpenWindow();

signals:
    void sigCreateVault(int state);

    void sigUnlocked(int state);

    void sigLocked(int state);

private:
    static quint64 winID;

public:
    static VaultHelper *instance();

private:
    explicit VaultHelper();
};

DPVAULT_END_NAMESPACE

#endif   // VAULTHELPER_H
