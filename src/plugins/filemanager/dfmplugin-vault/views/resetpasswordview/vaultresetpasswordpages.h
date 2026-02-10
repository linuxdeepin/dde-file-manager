// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTRESETPASSWORDPAGES_H
#define VAULTRESETPASSWORDPAGES_H

#include "dfmplugin_vault_global.h"
#include "../vaultpagebase.h"
#include "resetpasswordbyoldpasswordview.h"
#include "resetpasswordbykeyfileview.h"

namespace dfmplugin_vault {
class VaultResetPasswordPages : public VaultPageBase
{
    Q_OBJECT
public:
    explicit VaultResetPasswordPages(QWidget *parent = nullptr);
    ~VaultResetPasswordPages() override;

    void switchToOldPasswordView();
    void switchToKeyFileView();

public slots:
    void onButtonClicked(int index, const QString &text);
    void onSetBtnEnabled(int index, const bool &state);

private:
    ResetPasswordByOldPasswordView *oldPasswordView { nullptr };
    ResetPasswordByKeyFileView *keyFileView { nullptr };
    bool isOldPasswordView { true };
};
}
#endif   // VAULTRESETPASSWORDPAGES_H

