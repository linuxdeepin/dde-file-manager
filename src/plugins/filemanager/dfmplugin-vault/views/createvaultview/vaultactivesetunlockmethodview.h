// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTSETUNLOCKMETHODVIEW_H
#define VAULTSETUNLOCKMETHODVIEW_H

#include "dfmplugin_vault_global.h"
#include "vaultbaseview.h"

#include <dtkwidget_global.h>
#include <DSuggestButton>

#include <QWidget>

#define PASSWORD_LENGHT_MAX 24
#define TIPS_TIME 3600000

QT_BEGIN_NAMESPACE
class QGridLayout;
class QVBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DLabel;
class DLineEdit;
class DComboBox;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
inline constexpr int kPasswordLength { 18 };

class VaultActiveSetUnlockMethodView : public VaultBaseView
{
    Q_OBJECT
public:
    explicit VaultActiveSetUnlockMethodView(QWidget *parent = nullptr);
    ~VaultActiveSetUnlockMethodView();
    void clearText();
    void setEncryptInfo(EncryptInfo &info) override;

private slots:
    void slotPasswordEditing();
    void slotPasswordEditFinished();
    void slotPasswordEditFocusChanged(bool bFocus);
    void slotRepeatPasswordEditFinished();
    void slotRepeatPasswordEditing();
    void slotRepeatPasswordEditFocusChanged(bool bFocus);
    void slotGenerateEditChanged(const QString &str);
    //! 类型切换
    void slotTypeChanged(int index);
    //! 随即密码长度改变
    //!    void slotLengthChanged(int length);
    //! 限制密码的长度
    void slotLimiPasswordLength(const QString &passwordEdit);

private:
    void initUi();
    void initUiForSizeMode();
    void initConnect();
    //! 校验密码是否符合规则
    bool checkPassword(const QString &passwordEdit);
    //! 校验重复密码框是否符合规则
    bool checkRepeatPassword();
    //! 校验界面输入信息是否符合规则
    bool checkInputInfo();

private:
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DComboBox *typeCombo { nullptr };

    DTK_WIDGET_NAMESPACE::DLabel *passwordLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordEdit { nullptr };

    DTK_WIDGET_NAMESPACE::DLabel *repeatPasswordLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *repeatPasswordEdit { nullptr };

    DTK_WIDGET_NAMESPACE::DLabel *passwordHintLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLineEdit *tipsEdit { nullptr };

    DTK_WIDGET_NAMESPACE::DLabel *transEncryptionText { nullptr };
    QVBoxLayout *transEncryptTextLay { nullptr };

    DTK_WIDGET_NAMESPACE::DSuggestButton *nextBtn { nullptr };

    QGridLayout *gridLayout { nullptr };
};
}
#endif   //! VAULTSETUNLOCKMETHODVIEW_H
