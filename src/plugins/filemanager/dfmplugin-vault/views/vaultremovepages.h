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

class VaultRemovePages : public VaultPageBase
{
    Q_OBJECT
public:
    explicit VaultRemovePages(QWidget *parent = nullptr);
    ~VaultRemovePages() override {}

public slots:
    void onButtonClicked(int index);

    void onLockVault(int state);

    void onVaultRemoveFinish(bool result);

private slots:
    //! 异步授权时，此函数接收授权完成的结果
    void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

private:
    void initConnect();

    void showVerifyWidget();

    void showRemoveWidget();

    void setInfo(const QString &hintInfo);

protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private:
    VaultRemoveByPasswordView *passwordView { nullptr };
    VaultRemoveByRecoverykeyView *recoverykeyView { nullptr };
    VaultRemoveProgressView *progressView { nullptr };

    QStackedWidget *stackedWidget { nullptr };   //用于页面切换
    bool removeVault { false };

    QLabel *hintInfo { nullptr };
};
}
#endif   // VAULTREMOVEPAGES_H
