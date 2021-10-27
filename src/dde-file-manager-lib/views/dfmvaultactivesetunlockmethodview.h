/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
class QSlider;
class QComboBox;
class QGridLayout;
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
    // 随即密码长度改变
//    void slotLengthChanged(int length);
    // 限制密码的长度
    void slotLimiPasswordLength(const QString &password);
    // 随即生成密码按钮点击
//    void slotGeneratePasswordBtnClicked();

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

    QPushButton         *m_pNext;

    QGridLayout         *play1;

//    QLabel              *m_pLengthLabel;
//    QSlider             *m_pLengthSlider;
//    QLineEdit           *m_pLengthEdit;
//    QLabel              *m_pStrengthLabel;
//    QLabel              *m_pResultLabel;
//    QLineEdit           *m_pResultEdit;
//    QCheckBox           *m_pOtherMethod;
//    QPushButton         *m_pGenerateBtn;
};

#endif // DFMVAULTSETUNLOCKMETHODVIEW_H
