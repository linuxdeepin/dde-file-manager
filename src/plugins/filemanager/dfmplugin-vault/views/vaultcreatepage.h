// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    Q_OBJECT
public:
    explicit VaultActiveView(QWidget *parent = nullptr);

private slots:
    void slotNextWidget();
    void encryptVault();

private:
    void setBeginingState();
    void asyncCreateVault();
    Result createVault();
    bool handleKeyModeEncryption();
    bool handleTransparentModeEncryption();

private:
    //! 窗口容器
    QStackedWidget *stackedWidget { nullptr };

    VaultActiveStartView *startVaultWidget { nullptr };
    VaultActiveSetUnlockMethodView *setUnclockMethodWidget { nullptr };
    VaultActiveFinishedView *activeVaultFinishedWidget { nullptr };
    VaultActiveSaveKeyFileView *saveKeyFileWidget { nullptr };

    EncryptInfo encryptInfo;
};
}
#endif   //! VAULTACTIVEVIEW_H
