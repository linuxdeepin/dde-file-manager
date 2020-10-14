/**
 ** This file is part of the filemanager project.
 ** Copyright 2020 luzhen <luzhen@uniontech.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/
#include "dfmvaultpagebase.h"

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
    }
}

void DFMVaultPageBase::enterVaultDir()
{
    DFileManagerWindow *wnd = dynamic_cast<DFileManagerWindow *>(m_wndptr);
    if (wnd) {
        DUrl vaultUrl = VaultController::makeVaultUrl(VaultController::makeVaultLocalPath());
        wnd->cd(vaultUrl);
    }
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
}
