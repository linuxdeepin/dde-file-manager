/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
DPVAULT_BEGIN_NAMESPACE
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

private:
    QStackedWidget *stackedWidget { nullptr };

    //! retrieve password
    RetrievePasswordView *retrievePasswordView { nullptr };

    //! key unlock
    RecoveryKeyView *recoveryKeyView { nullptr };

    //! password unlock
    UnlockView *unlockView { nullptr };

    PasswordRecoveryView *passwordRecoveryView { nullptr };
};
DPVAULT_END_NAMESPACE
#endif   //!VAULTUNLOCKPAGES_H
