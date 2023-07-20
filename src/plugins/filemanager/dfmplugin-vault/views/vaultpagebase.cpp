// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultpagebase.h"
#include "utils/vaulthelper.h"

DPVAULT_USE_NAMESPACE

VaultPageBase::VaultPageBase(QWidget *parent)
    : DDialog(parent)
{
    this->moveToCenter();
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}
