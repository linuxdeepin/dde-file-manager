/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#include "dfmvaultpagebase.h"
#include "vault/vaulthelper.h"
#include "controllers/vaultcontroller.h"
#include "dfilemanagerwindow.h"

#include <QWindow>

DFMVaultPageBase::DFMVaultPageBase(QWidget *parent)
    : DDialog(parent)
{
    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
        this->moveToCenter();
    }
    // 修复BUG-45226 设置该弹窗为模态弹窗
    this->setModal(true);
}

void DFMVaultPageBase::enterVaultDir()
{
    //! bug 38585 这样修改可以减少一次文管是否设置双击文件始终在新窗口中打开
    DUrl vaultUrl = VaultController::makeVaultUrl(VaultController::makeVaultLocalPath());
    DFileManagerWindow *wnd = dynamic_cast<DFileManagerWindow *>(m_wndptr);
    AppController::instance()->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(wnd, DUrlList() << vaultUrl));
}

void DFMVaultPageBase::closeEvent(QCloseEvent *event)
{
    // 记录当前退出模态对话框状态
    VaultHelper::isModel = false;
    DDialog::closeEvent(event);
}

void DFMVaultPageBase::setWndPtr(QWidget *wnd)
{
    m_wndptr = wnd;
}

QWidget *DFMVaultPageBase::getWndPtr() const
{
    return m_wndptr;
}

void DFMVaultPageBase::showTop()
{
    this->activateWindow();
    this->showNormal();
    this->raise();
    // 记录当前处于模态弹窗状态
    VaultHelper::isModel = true;
}
