// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultpagebase.h"
#include "utils/vaulthelper.h"

using namespace dfmplugin_vault;

VaultPageBase::VaultPageBase(QWidget *parent)
    : DDialog(parent)
{
    this->moveToCenter();
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}
