// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleton.h"
#include "app/define.h"
#include "vault/interfaceactivevault.h"
#include "vaulthelper.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/dtaskdialog.h"
#include "controllers/vaultcontroller.h"

#include <DWindowManagerHelper>
#include <DForeignWindow>
#include <DSysInfo>

DFM_BEGIN_NAMESPACE

using namespace Dtk::Core;

// 初始化静态变量
bool VaultHelper::isModel = false;

VaultHelper::VaultHelper(QObject *parent) : QObject(parent)
{

}

bool VaultHelper::topVaultTasks()
{
    // 如果正在有保险箱的移动、粘贴、删除操作，置顶弹出任务框
    DTaskDialog *pTaskDlg = dialogManager->taskDialog();
    if (pTaskDlg) {
        if (pTaskDlg->haveNotCompletedVaultTask()) {
            // Flashing alert
            pTaskDlg->hide();
            pTaskDlg->showDialogOnTop();
            return true;
        }
    }
    // 如果当前有保险箱的压缩或解压缩任务，激活任务对话框进程
    QString strCmd = GET_COMPRESSOR_PID_SHELL(VAULT_BASE_PATH);
    QStringList lstShellOutput;
    // 执行shell命令，获得压缩进程PID
    int res = InterfaceActiveVault::executionShellCommand(strCmd, lstShellOutput);
    if (res == 0) { // shell命令执行成功
        QStringList::const_iterator itr = lstShellOutput.begin();
        QSet<QString> setResult;
        for (; itr != lstShellOutput.end(); ++itr) {
            setResult.insert(*itr);
        }
        if (setResult.count() > 0) { // 有压缩任务
            // 遍历桌面窗口
            bool bFlag = false;
            for (auto window : DWindowManagerHelper::instance()->currentWorkspaceWindows()) {
                QString strWid = QString("%1").arg(window->pid());
                // 如果当前窗口的进程PID属于压缩进程，则将窗口置顶
                if (setResult.contains(strWid)) {
                    window->raise();
                    bFlag = true;
                }
            }
            if (bFlag) {
                return true;
            }
        }
    } else {
        qDebug() << "Failed to execute PID search command!";
    }

    //! 如果正在有保险箱的移动、粘贴到桌面的任务，通知桌面进程置顶任务对话框
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.FileManager1",
                                                          "/org/freedesktop/FileManager1",
                                                          "org.freedesktop.FileManager1",
                                                          "topTaskDialog");
    // 修复BUG-44055 设置超时等待为1000毫米，提高用户操作流畅度
    QDBusMessage response = QDBusConnection::sessionBus().call(message, QDBus::Block, 1000);
    if (response.type() == QDBusMessage::ReplyMessage) {
        bool bValue = response.arguments().takeFirst().toBool();
        if (bValue) {
            return true;
        }
    }

    return false;
}

bool VaultHelper::killVaultTasks()
{
    //! 如果正在有保险箱的移动、粘贴、删除操作，强行结束任务
    DTaskDialog *pTaskDlg = dialogManager->taskDialog();
    if (pTaskDlg) {
        if (pTaskDlg->haveNotCompletedVaultTask()) {
            pTaskDlg->stopVaultTask();
        }
    }

    //! 如果当前有保险箱的压缩或解压缩任务，杀死任务对话框进程
    QString strCmd = GET_COMPRESSOR_PID_SHELL(VAULT_BASE_PATH);
    QStringList lstShellOutput;
    int res = InterfaceActiveVault::executionShellCommand(strCmd, lstShellOutput);
    if (res == 0) { //! shell命令执行成功
        QStringList::const_iterator itr = lstShellOutput.begin();
        for (; itr != lstShellOutput.end(); ++itr) {
            QString strCmd2 = QString("kill -9 %1").arg(*itr);
            QStringList lstShellOutput2;
            int res2 = InterfaceActiveVault::executionShellCommand(strCmd2, lstShellOutput2);
            if (res2 == 0)
                qDebug() << QString("杀死进程PID: %1 成功").arg(*itr);
        }
    } else {
        qDebug() << "执行查找进程PID命令失败!";
    }

    //! 如果正在有保险箱的移动、粘贴到桌面的任务，通知桌面进程置结束当前保险箱的任务
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.FileManager1",
                                                          "/org/freedesktop/FileManager1",
                                                          "org.freedesktop.FileManager1",
                                                          "closeTask");
    // 设置超时等待为1000毫秒
    QDBusMessage response = QDBusConnection::sessionBus().call(message, QDBus::Block, 1000);
    if (response.type() != QDBusMessage::ReplyMessage) {
        qDebug() << "close vault task failed!";
        return false;
    }

    return true;
}

bool VaultHelper::isVaultEnabled()
{
    if (!VaultController::ins()->isVaultVisiable())     // 判断域管策略,是否显示保险箱
        return false;

    // 获取系统类型
    DSysInfo::UosType uosType = DSysInfo::uosType();
    // 获取系统版本
    DSysInfo::UosEdition uosEdition = DSysInfo::uosEditionType();
    if (DSysInfo::UosServer == uosType) {   // 如果是服务器类型
        // 如果是企业版/行业版/欧拉版
        if (DSysInfo::UosEnterprise == uosEdition
                || DSysInfo::UosEnterpriseC == uosEdition
                || DSysInfo::UosEuler == uosEdition) {
            return true;
        }
    } else if (DSysInfo::UosDesktop == uosType) {   // 如果是桌面类型
        // 如果是专业版
        if (DSysInfo::UosProfessional == uosEdition) {
            return true;
        }
    }
    return false;
}

DFM_END_NAMESPACE
