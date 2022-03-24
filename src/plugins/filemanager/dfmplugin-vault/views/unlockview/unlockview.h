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
#ifndef UNLOCKVIEW_H
#define UNLOCKVIEW_H
#include "dfmplugin_vault_global.h"
#include "utils/vaultglobaldefine.h"

#include <DPasswordEdit>
#include <DToolTip>
#include <DFloatingWidget>
#include <DLabel>

#include <QFrame>
#include <QPushButton>

DPVAULT_BEGIN_NAMESPACE
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
    bool extraLockVault { true };
};
DPVAULT_END_NAMESPACE
#endif   // UNLOCKVIEW_H
