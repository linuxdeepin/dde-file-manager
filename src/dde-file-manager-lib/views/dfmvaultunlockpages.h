/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#pragma once

#include "dfmvaultpagebase.h"

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DToolTip;
class DFloatingWidget;
class DLabel;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultUnlockPages : public DFMVaultPageBase
{
    Q_OBJECT
public:
    static DFMVaultUnlockPages *instance();

    enum EN_ToolTip {
        Warning = 0,
        Information
    };

protected:
    // 重写视图隐藏事件，用于隐藏tooltip
    void hideEvent(QHideEvent *event) override;

public slots:
    void onButtonClicked(const int &index);

    void onPasswordChanged(const QString &pwd);

    void onVaultUlocked(int state);

    /**
     * @brief onReturnUnlockedPage 返回界面页面
     */
    void onReturnUnlockedPage();

protected:
    bool eventFilter(QObject *obj, QEvent *evt); //! m_forgetPassword 点击事件过滤处理

private slots:
    // 超时函数，定时隐藏tooltip显示
    void slotTooltipTimerTimeout();

private:
    explicit DFMVaultUnlockPages(QWidget *parent = nullptr);
    ~DFMVaultUnlockPages() override {}

    void showEvent(QShowEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

    void showToolTip(const QString &text, int duration, EN_ToolTip enType);

private:
    static DFMVaultUnlockPages *m_instance;
    DPasswordEdit *m_passwordEdit {nullptr};
    QPushButton *m_tipsButton {nullptr};
    bool m_bUnlockByPwd = false;

    DToolTip *m_tooltip {nullptr};
    DFloatingWidget *m_frame {nullptr};
    // 定时器，用于定时隐藏tooltip
    QTimer *pTooltipTimer {nullptr};
    DLabel *m_forgetPassword {nullptr};  //! 忘记密码提示与找回入口
    DFMVaultPageBase *m_retrievePage {nullptr};  //! 找回密码验证页面
    bool m_extraLockVault = true;
};
