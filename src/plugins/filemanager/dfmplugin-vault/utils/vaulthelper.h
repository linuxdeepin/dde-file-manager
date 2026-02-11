// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHELPER_H
#define VAULTHELPER_H

#include "dfmplugin_vault_global.h"
#include "utils/vaultdefine.h"

#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/dfm_global_defines.h>
#include <DMenu>

#include <QObject>
#include <QIcon>
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

    QUrl sourceRootUrlWithSlash();

    QUrl pathToVaultVirtualUrl(const QString &path);

    VaultState state(const QString &baseDir, bool useCache = true) const;
    bool updateState(VaultState curState);

    /*!
     * \brief getVaultVersion   获取当前保险箱版本是否是1050及以上版本
     * \return  true大于等于1050,false小于1050
     */
    bool getVaultVersion() const;

    void killVaultTasks();

    quint64 currentWindowId();

    void appendWinID(const quint64 &winId);
    void removeWinID(const quint64 &winId);

    bool enableUnlockVault();

public:
    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

    static void siderItemClicked(quint64 windowId, const QUrl &url);

    static DTK_WIDGET_NAMESPACE::DMenu *createMenu();

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

    bool unlockVault(const QString &password);

    bool lockVault(bool isForced);

    void defaultCdAction(const quint64 windowId, const QUrl &url);

    void openNewWindow(const QUrl &url);

    void createVaultDialog();

    void unlockVaultDialog();

    void showRemoveVaultDialog();

    void showResetPasswordDialog();

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
