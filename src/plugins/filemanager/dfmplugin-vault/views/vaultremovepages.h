// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEPAGES_H
#define VAULTREMOVEPAGES_H

#include "dfmplugin_vault_global.h"

#include "vaultpagebase.h"

#include <polkit-qt5-1/PolkitQt1/Authority>

class QStackedWidget;

namespace dfmplugin_vault {

class VaultRemoveProgressView;
class VaultRemoveByRecoverykeyView;
class VaultRemoveByPasswordView;
class VaultRemoveByTpmPinWidget;
class VaultRemoveByNoneWidget;

class VaultRemovePages : public VaultPageBase
{
    Q_OBJECT
public:
    explicit VaultRemovePages(QWidget *parent = nullptr);
    ~VaultRemovePages() override {}

public Q_SLOTS:
    void pageSelect(RemoveWidgetType type);
    void onButtonClicked(int index, const QString &text);
    void setBtnEnable(int index, bool enable);

private:
    void initUI();
    void initConnect();

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    void showPasswordWidget();
    void showRecoveryKeyWidget();
    void showRemoveProgressWidget();
    void showTpmPinWidget();
    void showNodeWidget();

    VaultRemoveByPasswordView *passwordView { Q_NULLPTR };
    VaultRemoveByRecoverykeyView *recoverykeyView { Q_NULLPTR };
    VaultRemoveProgressView *progressView { Q_NULLPTR };
    VaultRemoveByTpmPinWidget *tpmPinWidget { Q_NULLPTR };
    VaultRemoveByNoneWidget *noneWidget { Q_NULLPTR };
    bool removeVault { false };
};
}
#endif   // VAULTREMOVEPAGES_H
