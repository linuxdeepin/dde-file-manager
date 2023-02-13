// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTACTIVEVIEW_H
#define VAULTACTIVEVIEW_H

#include "dfmplugin_vault_global.h"

#include "vaultpagebase.h"

class QStackedWidget;
class QMouseEvent;

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
    //! 重写基类关闭事件
    void closeEvent(QCloseEvent *event) override;

    void showEvent(QShowEvent *event) override;

private slots:
    void slotNextWidget();

private:
    void setBeginingState();

private:
    //! 窗口容器
    QStackedWidget *stackedWidget { nullptr };

    VaultActiveStartView *startVaultWidget { nullptr };
    VaultActiveSetUnlockMethodView *setUnclockMethodWidget { nullptr };
    VaultActiveFinishedView *activeVaultFinishedWidget { nullptr };
    VaultActiveSaveKeyFileView *saveKeyFileWidget { nullptr };
};
}
#endif   //! VAULTACTIVEVIEW_H
