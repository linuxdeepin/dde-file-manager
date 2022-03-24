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
#ifndef PASSWORDRECOVERYPAGE_H
#define PASSWORDRECOVERYPAGE_H
#include "dfmplugin_vault_global.h"
#include "utils/vaultglobaldefine.h"

#include <QFrame>

#include <DLabel>
DPVAULT_BEGIN_NAMESPACE
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
DPVAULT_END_NAMESPACE
#endif   // PASSWORDRECOVERYPAGE_H
