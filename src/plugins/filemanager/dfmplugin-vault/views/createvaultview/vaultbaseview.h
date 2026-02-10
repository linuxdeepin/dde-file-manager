// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTBASEVIEW_H
#define VAULTBASEVIEW_H

#include "dfmplugin_vault_global.h"

#include <QWidget>

namespace dfmplugin_vault {
class VaultBaseView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultBaseView(QWidget *parent = nullptr);

    virtual void setEncryptInfo(EncryptInfo &info);

Q_SIGNALS:
    void accepted();
};
}

#endif   // VAULTBASEVIEW_H
