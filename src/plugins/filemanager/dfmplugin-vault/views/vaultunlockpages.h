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

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DToolTip;
class DFloatingWidget;
class DLabel;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE
DPVAULT_BEGIN_NAMESPACE
class VaultUnlockPages : public VaultPageBase
{
    Q_OBJECT
public:
    enum EN_ToolTip {
        Warning = 0,
        Information
    };

    explicit VaultUnlockPages(QWidget *parent = nullptr);
    ~VaultUnlockPages() override {}

public slots:
    void onButtonClicked(const int &index);

    void onPasswordChanged(const QString &pwd);

    void onVaultUlocked(int state);

    /*!
     * /brief onReturnUnlockedPage 返回界面页面
     */
    void onReturnUnlockedPage();

private slots:
    //! 超时函数，定时隐藏tooltip显示
    void slotTooltipTimerTimeout();

protected:
    virtual void showEvent(QShowEvent *event) override;

    virtual void closeEvent(QCloseEvent *event) override;

    void showToolTip(const QString &text, int duration, EN_ToolTip enType);

    //! 重写视图隐藏事件，用于隐藏tooltip
    virtual void hideEvent(QHideEvent *event) override;

    virtual bool eventFilter(QObject *obj, QEvent *evt) override;   //! m_forgetPassword 点击事件过滤处理

private:
    DPasswordEdit *passwordEdit { nullptr };
    QPushButton *tipsButton { nullptr };
    bool unlockByPwd { false };

    DToolTip *tooltip { nullptr };
    DFloatingWidget *floatWidget { nullptr };
    //! 定时器，用于定时隐藏tooltip
    QTimer *tooltipTimer { nullptr };
    DLabel *forgetPassword { nullptr };   //! 忘记密码提示与找回入口
    VaultPageBase *retrievePage { nullptr };   //! 找回密码验证页面
    bool extraLockVault { true };
};
DPVAULT_END_NAMESPACE
#endif   //!VAULTUNLOCKPAGES_H
