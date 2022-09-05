// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    explicit DFMVaultUnlockPages(QWidget *parent = nullptr);
    ~DFMVaultUnlockPages() override {}

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
