// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultbaseview.h"

using namespace dfmplugin_vault;

VaultBaseView::VaultBaseView(QWidget *parent)
    : QWidget(parent)
{
}

void VaultBaseView::setEncryptInfo(EncryptInfo &info)
{
    Q_UNUSED(info)
}
