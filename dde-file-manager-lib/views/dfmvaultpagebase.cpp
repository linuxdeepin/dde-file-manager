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

DFMVaultPageBase::DFMVaultPageBase(QWidget *parent)
    : DDialog(parent)
{
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
