// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVESTARTVIEW_H
#define VAULTACTIVESTARTVIEW_H

#include "dfmplugin_vault_global.h"

#include <DPushButton>
#include <DLabel>
#include <DSuggestButton>

#include <QWidget>

namespace dfmplugin_vault {
class VaultActiveStartView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultActiveStartView(QWidget *parent = nullptr);

signals:
    void sigAccepted();

private slots:
    void slotStartBtnClicked();

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
