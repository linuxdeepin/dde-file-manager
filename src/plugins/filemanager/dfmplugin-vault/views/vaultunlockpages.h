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
#include "unlockview/unlockwidgetfortpm.h"

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

public slots:
    void pageSelect(PageType page);
    void onButtonClicked(int index, const QString &text);
    void onSetBtnEnabled(int index, bool state);
    void setAllowClose(bool value);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void showUnlockByTpmWidget();

    QStackedWidget *stackedWidget { nullptr };
    RetrievePasswordView *retrievePasswordView { nullptr };
    RecoveryKeyView *recoveryKeyView { nullptr };
    UnlockView *unlockView { nullptr };
    PasswordRecoveryView *passwordRecoveryView { nullptr };
    UnlockWidgetForTpm *unlockByTpmWidget { Q_NULLPTR };
    bool allowClose { true };
};
}
#endif   //!VAULTUNLOCKPAGES_H
