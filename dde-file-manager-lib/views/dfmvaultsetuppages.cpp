/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmvaultsetuppages.h"

#include <QLabel>

DFM_BEGIN_NAMESPACE

DFMVaultSetupPages::DFMVaultSetupPages(QWidget *parent)
    : DFMVaultPages(parent)
{
    insertPage("welcome", new QLabel("Welcome", this));
    insertPage("set_password", new QLabel("Set Password", this));
}

QPair<DUrl, bool> DFMVaultSetupPages::requireRedirect(VaultController::VaultState state)
{
    if (state == VaultController::Unlocked) {
        return {VaultController::makeVaultUrl(), true};
    }
    return DFMVaultPages::requireRedirect(state);
}

QString DFMVaultSetupPages::pageString(const DUrl &url)
{
    // welcome -> set_password -> ask_create_recovery_key
    if (url.path() == "/set_password") {
        return "set_password";
    }
    return "welcome";
}

DFM_END_NAMESPACE
