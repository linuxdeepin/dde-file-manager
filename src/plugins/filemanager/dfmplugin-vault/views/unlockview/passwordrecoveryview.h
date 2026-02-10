// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PASSWORDRECOVERYPAGE_H
#define PASSWORDRECOVERYPAGE_H
#include "dfmplugin_vault_global.h"
#include "utils/vaultdefine.h"

#include <QFrame>

#include <DLabel>
namespace dfmplugin_vault {
class PasswordRecoveryView : public QFrame
{
    Q_OBJECT
public:
    explicit PasswordRecoveryView(QWidget *parent = nullptr);
    virtual ~PasswordRecoveryView();

    void initUI();

    QStringList btnText();

    QString titleText();

    /*!
     * /brief setResultsPage    设置找回密码后的页面
     * /param password          密码
     */
    void setResultsPage(QString password);

    /*!
     * /brief onButtonClicked 返回解锁页面与验证密钥按钮
     * /param index         按钮编号
     * /param text          按钮文本
     */
    void buttonClicked(int index, const QString &text);

signals:
    /*!
     * /brief signalJump 跳转页面
     */
    void signalJump(const PageType type);

    void sigBtnEnabled(const int &index, const bool &state);

    void sigCloseDialog();

private:
    //! 密钥验证成页面标题
    DTK_WIDGET_NAMESPACE::DLabel *verificationPageTitle { nullptr };
    //! 找回的密码信息
    DTK_WIDGET_NAMESPACE::DLabel *passwordMsg { nullptr };
    //! 安全提示
    DTK_WIDGET_NAMESPACE::DLabel *hintMsg { nullptr };
};
}
#endif   // PASSWORDRECOVERYPAGE_H
