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
#include "services/filemanager/fileencrypt/fileencryptservice.h"
#include "services/filemanager/sidebar/sidebarservice.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/computer/computerservice.h"
#include "services/filemanager/titlebar/titlebarservice.h"
#include "services/filemanager/windows/windowsservice.h"
#include "services/common/propertydialog/property_defines.h"
//#include "services/common/propertydialog/propertydialogservice.h"
#include "services/common/fileoperations/fileoperationsservice.h"

#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"

#include <QObject>
#include <QIcon>
#include <QMenu>
#include <QTimer>

DPVAULT_BEGIN_NAMESPACE
class VaultHelper final : public QObject
{
    Q_OBJECT
public:
    //! 保险箱当前页面标记
    enum VaultPageMark {
        kUnknown,
        kCreateVaultPage,
        kCreateVaultPage1,
        kUnlockVaultPage,
        kRetrievePassWordPage,
        kDeleteFilePage,
        kDeleteVaultPage,
        kCopyFilePage,
        kClipboardPage,
        kVaultPage
    };

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

    /*!
    * \brief isVaultEnabled
    * \return true vault is available, vice versa.
    */
    bool isVaultEnabled();

    VaultState state(QString lockBaseDir);

    QString makeVaultLocalPath(QString path = "", QString base = "");

    /*!
     * \brief getVaultVersion   获取当前保险箱版本是否是1050及以上版本
     * \return  true大于等于1050,false小于1050
     */
    bool getVaultVersion();

    QString vaultLockPath();

    QString vaultUnlockPath();

    /*!
     * \brief getVaultPolicy 获取当前策略
     * \return 返回保险箱是否隐藏  1隐藏 2显示
     */
    int getVaultPolicy();

    /*!
     * \brief setVaultPolicyState 设置策略是否可用
     * \param policyState 1策略可用 2策略不可用
     * \return 设置是否成功
     */
    bool setVaultPolicyState(int policyState);

    /*!
     * \brief  获取当前所处保险箱页面
     * \return 返回当前页面标识
     */
    VaultPageMark getVaultCurrentPageMark();

    /*!
     * \brief 设置当前所处保险箱页面
     * \param mark 页面标识
     */
    void setVauleCurrentPageMark(VaultPageMark mark);

    /*!
     * \brief isVaultVisiable 获取保险箱显示状态
     * \return true显示、false隐藏
     */
    bool isVaultVisiable();

    void removeSideBarVaultItem();

    void removeComputerVaultItem();

    void killVaultTasks();

    quint64 currentWindowId();

    void appendWinID(quint64 id);

public:
    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);

    static void siderItemClicked(quint64 windowId, const QUrl &url);

    static DSB_FM_NAMESPACE::FileEncryptService *fileEncryptServiceInstance();

    static DSB_FM_NAMESPACE::SideBarService *sideBarServiceInstance();

    static DSB_FM_NAMESPACE::WindowsService *windowServiceInstance();

    static DSB_FM_NAMESPACE::ComputerService *computerServiceInstance();

    static DSB_FM_NAMESPACE::TitleBarService *titleBarServiceInstance();

    static DSB_FM_NAMESPACE::WorkspaceService *workspaceServiceInstance();

    static DSC_NAMESPACE::FileOperationsService *fileOperationsServIns();

    static QMenu *createMenu();

    static QWidget *createVaultPropertyDialog(const QUrl &url);

    static bool openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error);

    static bool writeToClipBoardHandle(const quint64 windowId,
                                       const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                       const QList<QUrl> urls);

    static JobHandlePointer moveToTrashHandle(const quint64 windowId, const QList<QUrl> sources,
                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static JobHandlePointer deletesHandle(const quint64 windowId, const QList<QUrl> sources,
                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static JobHandlePointer copyHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                       const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static JobHandlePointer cutHandle(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

    static bool mkdirHandle(const quint64 windowId, const QUrl url, QString *error, const DFMBASE_NAMESPACE::Global::CreateFileType type);

    static bool touchFileHandle(const quint64 windowId, const QUrl url, QString *error, const DFMBASE_NAMESPACE::Global::CreateFileType type);

    static bool renameHandle(const quint64 windowId, const QUrl oldUrl, const QUrl newUrl, QString *);

    static QUrl vaultToLocalUrl(const QUrl &url);

    static VaultHelper *instance();

    static QMap<DSC_NAMESPACE::CPY_NAMESPACE::BasicExpandType, DSC_NAMESPACE::CPY_NAMESPACE::BasicExpand> basicViewFieldFunc(const QUrl &url);

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

    //! 保险箱策略处理函数
    void slotVaultPolicy();

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

    //! 用于记录当前保险箱所处页面标识
    VaultPageMark recordVaultPageMark;
    bool vaultVisiable { true };
};

DPVAULT_END_NAMESPACE

#endif   // VAULTHELPER_H
