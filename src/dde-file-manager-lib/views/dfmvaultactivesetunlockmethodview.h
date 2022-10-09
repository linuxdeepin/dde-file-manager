// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMVAULTSETUNLOCKMETHODVIEW_H
#define DFMVAULTSETUNLOCKMETHODVIEW_H

#include <dtkwidget_global.h>

#include <QWidget>

#define PASSWORD_LENGHT_MAX     24
#define TIPS_TIME               3600000

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;
class OperatorCenter;
class QComboBox;
class QGridLayout;
class QVBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DLabel;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultActiveSetUnlockMethodView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultActiveSetUnlockMethodView(QWidget *parent = nullptr);
    ~DFMVaultActiveSetUnlockMethodView();
    void clearText();

signals:
    void sigAccepted();
    // 抛出二维码显示的内容
    void qrcodeContant(const QString &qrcode);

public slots:

private slots:
    void slotPasswordEditing();
    void slotPasswordEditFinished();
    void slotPasswordEditFocusChanged(bool bFocus);
    void slotRepeatPasswordEditFinished();
    void slotRepeatPasswordEditing();
    void slotRepeatPasswordEditFocusChanged(bool bFocus);
    void slotGenerateEditChanged(const QString &str);
    // 下一步按钮点击
    void slotNextBtnClicked();
    // 类型切换
    void slotTypeChanged(int index);
    // 限制密码的长度
    void slotLimiPasswordLength(const QString &password);

private:
    // 校验密码是否符合规则
    bool checkPassword(const QString &password);
    // 校验重复密码框是否符合规则
    bool checkRepeatPassword();
    // 校验界面输入信息是否符合规则
    bool checkInputInfo();

    void showEvent(QShowEvent *event) override;

private:
    QComboBox           *m_pTypeCombo;

    DLabel              *m_pPasswordLabel;
    DPasswordEdit       *m_pPassword;

    DLabel              *m_pRepeatPasswordLabel;
    DPasswordEdit       *m_pRepeatPassword;

    DLabel              *m_pPasswordHintLabel;
    QLineEdit           *m_pTips;

    DLabel              *TransparentEncryptionText;
    QVBoxLayout         *textLay;

    QPushButton         *m_pNext;

    QGridLayout         *play1;
};

#endif // DFMVAULTSETUNLOCKMETHODVIEW_H
