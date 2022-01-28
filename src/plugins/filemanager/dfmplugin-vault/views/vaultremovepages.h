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

#ifndef VAULTREMOVEPAGES_H
#define VAULTREMOVEPAGES_H

#include "dfmplugin_vault_global.h"

#include "vaultpagebase.h"

#include <polkit-qt5-1/PolkitQt1/Authority>

class QStackedWidget;

DPVAULT_BEGIN_NAMESPACE

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

    void onVualtRemoveFinish(bool result);

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
DPVAULT_END_NAMESPACE
#endif   // VAULTREMOVEPAGES_H
