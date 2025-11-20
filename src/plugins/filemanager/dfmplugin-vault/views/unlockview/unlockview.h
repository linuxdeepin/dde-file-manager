// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNLOCKVIEW_H
#define UNLOCKVIEW_H
#include "dfmplugin_vault_global.h"
#include "utils/vaultdefine.h"

#include <DPasswordEdit>
#include <DToolTip>
#include <DFloatingWidget>
#include <DLabel>
#include <DSpinner>

#include <QFrame>
#include <QPushButton>
#include <QFutureWatcher>

namespace dfmplugin_vault {
class UnlockView : public QFrame
{
    Q_OBJECT
public:
    enum ENToolTip {
        kWarning = 0,
        kInformation
    };
    explicit UnlockView(QWidget *parent = nullptr);
    virtual ~UnlockView() override;

    QStringList btnText();

    QString titleText();

    void buttonClicked(int index, const QString &text);

signals:
    /*!
     * /brief signalJump 页面跳转
     */
    void signalJump(const PageType type);

    void sigBtnEnabled(const int &index, const bool &state);

    void sigCloseDialog();

public slots:

    void onPasswordChanged(const QString &pwd);

    void onVaultUlocked(int state);

    void onPasswordCheckFinished();

private slots:
    //! UI初始化
    void initUI();

    //! 超时函数，定时隐藏tooltip显示
    void slotTooltipTimerTimeout();

protected:
    virtual void showEvent(QShowEvent *event) override;

    virtual void closeEvent(QCloseEvent *event) override;

    void showToolTip(const QString &text, int duration, ENToolTip enType);

    //! 重写视图隐藏事件，用于隐藏tooltip
    virtual void hideEvent(QHideEvent *event) override;

    virtual bool eventFilter(QObject *obj, QEvent *evt) override;   //! m_forgetPassword 点击事件过滤处理

private:
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordEdit { nullptr };
    QPushButton *tipsButton { nullptr };
    bool unlockByPwd { false };

    DTK_WIDGET_NAMESPACE::DToolTip *tooltip { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *floatWidget { nullptr };
    //! 定时器，用于定时隐藏tooltip
    QTimer *tooltipTimer { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *forgetPassword { nullptr };   //! 忘记密码提示与找回入口
    DTK_WIDGET_NAMESPACE::DSpinner *spinner { nullptr };   //! 加载动画
    struct PasswordCheckResult {
        bool isValid;
        QString masterKey;   // Base64编码的主密钥
    };
    QFutureWatcher<PasswordCheckResult> *passwordCheckWatcher { nullptr };   //! 密码验证异步操作
    QString pendingPassword;   //! 待验证的密码
    bool extraLockVault { true };
};
}
#endif   // UNLOCKVIEW_H
