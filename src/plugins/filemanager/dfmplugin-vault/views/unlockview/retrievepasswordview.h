// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTRETRIEVEPASSWORD_H
#define VAULTRETRIEVEPASSWORD_H

#include "dfmplugin_vault_global.h"
#include "utils/vaultdefine.h"

#include <DFileChooserEdit>
#include <DLabel>
#include <DFileDialog>

#include <QComboBox>

namespace dfmplugin_vault {
class RetrievePasswordView : public QFrame
{
    Q_OBJECT
public:
    explicit RetrievePasswordView(QWidget *parent = nullptr);

    /*!
     * /brief verificationKey 验证密钥
     */
    void verificationKey();

    /*!
     * /brief getUserName   获取用户名
     * /return  返回当前用户的用户名
     */
    QString getUserName();

    QStringList btnText();

    QString titleText();

    /*!
     * /brief onButtonClicked 返回解锁页面与验证密钥按钮
     * /param index         按钮编号
     * /param text          按钮文本
     */
    void buttonClicked(int index, const QString &text);

    QString ValidationResults();

signals:
    /*!
     * /brief signalJump 页面跳转
     */
    void signalJump(const PageType type);

    void sigBtnEnabled(const int &index, const bool &state);

    void sigResults(QString message);

public slots:

    /*!
     * /brief onComboBoxIndex   选择密钥文件的方式
     * /param index             0默认路径选择，1用户自己选择
     */
    void onComboBoxIndex(int index);

    /*!
     * /brief onBtnSelectFilePath   设置用户选择的密钥文件路径
     * /param path                  选择的密钥文件路径
     */
    void onBtnSelectFilePath(const QString &path);

private slots:
    /*!
     * /brief slotCheckAuthorizationFinished 异步授权时，此函数接收授权完成的结果
     * /param result 授权结果 成功或失败
     */
    void slotCheckAuthorizationFinished(bool result);

private:
    /*!
     * /brief setVerificationPage   设置密钥验证页面
     */
    void setVerificationPage();

protected:
    void showEvent(QShowEvent *event) override;

private:
    //! 找回密码页面标题
    DTK_WIDGET_NAMESPACE::DLabel *currentPageTitle { nullptr };

    //! 选择要验证的密钥路径
    QComboBox *savePathTypeComboBox { nullptr };

    //! 用户自选密钥文件路径编辑框
    DTK_WIDGET_NAMESPACE::DFileChooserEdit *filePathEdit { nullptr };

    //! 默认密钥文件路径编辑框
    QLineEdit *defaultFilePathEdit { nullptr };

    //! 密钥验证失败提示标签
    DTK_WIDGET_NAMESPACE::DLabel *verificationPrompt { nullptr };

    //! 选择密钥页面
    QFrame *selectKeyPage { nullptr };

    //! 4个按钮的文本
    QStringList btnList {};

    QString validationResults;

    QGridLayout *funLayout { nullptr };
};
}
#endif   // VAULTRETRIEVEPASSWORD_H
