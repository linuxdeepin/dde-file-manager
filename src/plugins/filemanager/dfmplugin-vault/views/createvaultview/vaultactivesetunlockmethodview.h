// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTSETUNLOCKMETHODVIEW_H
#define VAULTSETUNLOCKMETHODVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QWidget>

#define PASSWORD_LENGHT_MAX 24
#define TIPS_TIME 3600000

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QCheckBox;
class QLabel;
class OperatorCenter;
class QSlider;
class QComboBox;
class QGridLayout;
class QVBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DLabel;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE
namespace dfmplugin_vault {
inline constexpr int kPasswordLength { 18 };

class VaultActiveSetUnlockMethodView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultActiveSetUnlockMethodView(QWidget *parent = nullptr);
    void clearText();

signals:
    void sigAccepted();

public slots:

private slots:
    void slotPasswordEditing();
    void slotPasswordEditFinished();
    void slotPasswordEditFocusChanged(bool bFocus);
    void slotRepeatPasswordEditFinished();
    void slotRepeatPasswordEditing();
    void slotRepeatPasswordEditFocusChanged(bool bFocus);
    void slotGenerateEditChanged(const QString &str);
    //! 下一步按钮点击
    void slotNextBtnClicked();
    //! 类型切换
    void slotTypeChanged(int index);
    //! 随即密码长度改变
    //!    void slotLengthChanged(int length);
    //! 限制密码的长度
    void slotLimiPasswordLength(const QString &passwordEdit);

private:
    //! 校验密码是否符合规则
    bool checkPassword(const QString &passwordEdit);
    //! 校验重复密码框是否符合规则
    bool checkRepeatPassword();
    //! 校验界面输入信息是否符合规则
    bool checkInputInfo();

    void showEvent(QShowEvent *event) override;

private:
    QComboBox *typeCombo { nullptr };

    DLabel *passwordLabel { nullptr };
    DPasswordEdit *passwordEdit { nullptr };

    DLabel *repeatPasswordLabel { nullptr };
    DPasswordEdit *repeatPasswordEdit { nullptr };

    DLabel *passwordHintLabel { nullptr };
    QLineEdit *tipsEdit { nullptr };

    DLabel *transEncryptionText { nullptr };
    QVBoxLayout *transEncryptTextLay { nullptr };

    QPushButton *nextBtn { nullptr };

    QGridLayout *gridLayout { nullptr };
};
}
#endif   //! VAULTSETUNLOCKMETHODVIEW_H
