// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVEVIEW_H
#define VAULTACTIVEVIEW_H

#include "dfmplugin_vault_global.h"
#include "vaultpagebase.h"

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QMouseEvent;
QT_END_NAMESPACE

namespace dfmplugin_vault {
class VaultActiveStartView;
class VaultActiveSetUnlockMethodView;
class VaultActiveFinishedView;
class VaultActiveSaveKeyFileView;

class VaultActiveView : public VaultPageBase
{
public:
    explicit VaultActiveView(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void slotNextWidget();
    void setAllowClose(bool value);

private:
    QStackedWidget *stackedWidget { nullptr };
    VaultActiveStartView *startVaultWidget { nullptr };
    VaultActiveSetUnlockMethodView *setUnclockMethodWidget { nullptr };
    VaultActiveFinishedView *activeVaultFinishedWidget { nullptr };
    VaultActiveSaveKeyFileView *saveKeyFileWidget { nullptr };
    bool allowClose { true };
};
}
#endif   //! VAULTACTIVEVIEW_H
