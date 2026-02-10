// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVESTARTVIEW_H
#define VAULTACTIVESTARTVIEW_H

#include "dfmplugin_vault_global.h"
#include "vaultbaseview.h"

#include <DPushButton>
#include <DLabel>
#include <DSuggestButton>

#include <QWidget>

namespace dfmplugin_vault {
class VaultActiveStartView : public VaultBaseView
{
    Q_OBJECT
public:
    explicit VaultActiveStartView(QWidget *parent = nullptr);

private:
    void initUi();
    void initUiForSizeMode();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DSuggestButton *startBtn { nullptr };   //! 开启包厢按钮
};
}
#endif   // VAULTACTIVESTARTVIEW_H
