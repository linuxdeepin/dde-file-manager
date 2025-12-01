// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTUNLOCKPAGES_H
#define VAULTUNLOCKPAGES_H

#include "dfmplugin_vault_global.h"

#include "vaultpagebase.h"
#include "unlockview/unlockview.h"
#include "unlockview/retrievepasswordview.h"
#include "unlockview/recoverykeyview.h"
#include "unlockview/passwordrecoveryview.h"
#include "utils/vaultdefine.h"

#include <QStackedWidget>

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DToolTip;
class DFloatingWidget;
class DLabel;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE
namespace dfmplugin_vault {
class VaultRetrievePassword;
class VaultUnlockPages : public VaultPageBase
{
    Q_OBJECT
public:
    explicit VaultUnlockPages(QWidget *parent = nullptr);
    ~VaultUnlockPages() override;

private:
    void initUI();

public slots:
    void pageSelect(PageType page);

    void onButtonClicked(int index, const QString &text);

    void onSetBtnEnabled(int index, const bool &state);

    // 老密码方案迁移模式（从pbkdf2迁移到LUKS）
    void setOldPasswordSchemeMigrationMode(bool enabled);
    bool isOldPasswordSchemeMigrationMode() const;

private:
    QStackedWidget *stackedWidget { nullptr };

    bool isOldPasswordSchemeMigrationModeFlag { false };

    //! retrieve password
    RetrievePasswordView *retrievePasswordView { nullptr };

    //! key unlock
    RecoveryKeyView *recoveryKeyView { nullptr };

    //! password unlock
    UnlockView *unlockView { nullptr };

    PasswordRecoveryView *passwordRecoveryView { nullptr };
};
}
#endif   //!VAULTUNLOCKPAGES_H
