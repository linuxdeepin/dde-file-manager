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
#include "utils/vaultdefine.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/dfm_global_defines.h"

#include <QObject>
#include <QIcon>
#include <QMenu>
#include <QTimer>

namespace dfmplugin_vault {
class VaultHelper final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultHelper)
public:
    inline QString scheme()
    {
        return "dfmvault";
    }

    inline QIcon icon()
    {
        return QIcon::fromTheme("drive-harddisk-encrypted-symbolic");
    }

    QUrl rootUrl();

    QUrl sourceRootUrl();

    QUrl pathToVaultVirtualUrl(const QString &path);

    VaultState state(QString lockBaseDir);

    /*!
     * \brief getVaultVersion   获取当前保险箱版本是否是1050及以上版本
     * \return  true大于等于1050,false小于1050
     */
    bool getVaultVersion();

    void killVaultTasks();

    quint64 currentWindowId();

    void appendWinID(const quint64 &winId);
    void removeWinID(const quint64 &winId);

public:
    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

    static void siderItemClicked(quint64 windowId, const QUrl &url);

    static QMenu *createMenu();

    static QWidget *createVaultPropertyDialog(const QUrl &url);

    static QUrl vaultToLocalUrl(const QUrl &url);

    static VaultHelper *instance();

    static void recordTime(const QString &group, const QString &key);
    static bool isVaultFile(const QUrl &url);

    bool urlsToLocal(const QList<QUrl> &origins, QList<QUrl> *urls);

    void showInProgressDailog(QString progressState);

public slots:
    void slotlockVault(int state);

    void createVault(QString &password);

    void unlockVault(QString &password);

    void lockVault();

    void defaultCdAction(const quint64 windowId, const QUrl &url);

    void openNewWindow(const QUrl &url);

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

    //! 通知关闭对话框
    void sigCloseWindow();

private:
    explicit VaultHelper();

private:
    QList<quint64> winIDs {};

    quint64 currentWinID { 0 };
};

}

#endif   // VAULTHELPER_H
