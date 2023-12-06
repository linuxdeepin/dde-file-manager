// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTSETUNLOCKMETHODVIEW_H
#define VAULTSETUNLOCKMETHODVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>
#include <DPushButton>

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

class VaultActiveSetUnlockMethodView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultActiveSetUnlockMethodView(bool tpmAvailabel = false, QWidget *parent = nullptr);

signals:
    void sigAccepted();

private slots:
    void slotPasswordEditing();
    void slotPasswordEditFinished();
    void slotPasswordEditFocusChanged(bool bFocus);
    void slotRepeatPasswordEditFinished();
    void tpmPinEditFinished();
    void repeatPinEditFinished();
    void slotRepeatPasswordEditing();
    void slotRepeatPasswordEditFocusChanged(bool bFocus);
    void slotGenerateEditChanged(const QString &str);
    void slotNextBtnClicked();
    void slotTypeChanged(int index);
    void slotLimiPasswordLength(const QString &passwordEdit);

private:
    void initUi();
    void initKeyEncryptWidget();
    void initTransEncryptWidget();
    void initTPMWithoutPinEncryptWidget();
    void initTPMWithPinEncryptWidget();
    void initConnect();
    bool checkPassword(const QString &passwordEdit);
    bool checkRepeatPassword();
    bool checkRepeatPin();
    bool checkTPMPin(const QString &pinCode);
    bool checkTPMAlgo();
    bool checkInputInfo();
    void removeEncryptWidgetByType(const QString &type);
    void addEncryptWidgetByType(const QString &type);
    bool preprocessKeyEncrypt();
    bool preprocessTranslateEncrypt();
    bool preprocessTpmWithoutPinEncrypt();
    bool preprocessTpmWithPinEncrypt();

protected:
    void showEvent(QShowEvent *event) override;

private:
    DTK_WIDGET_NAMESPACE::DComboBox *typeCombo { Q_NULLPTR };

    QWidget *keyEncryptWidget { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *passwordLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordEdit { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *repeatPasswordLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *repeatPasswordEdit { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *passwordHintLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLineEdit *tipsEdit { Q_NULLPTR };

    QWidget *transEncryptWidget { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *transEncryptionText { Q_NULLPTR };

    QWidget *tpmWithoutPinEncryptWidget { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *tpmWithoutPinText { Q_NULLPTR };

    QWidget *tpmWithPinEncryptWidget { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *tpmPinLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *tpmPinEdit { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *repeatPinLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *repeatPinEdit { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLabel *tpmPinHintLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DLineEdit *tpmPinHintEdit { Q_NULLPTR };

    DTK_WIDGET_NAMESPACE::DLabel *errorLabel { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DPushButton *nextBtn { Q_NULLPTR };
    QGridLayout *gridLayout { Q_NULLPTR };

    int curTypeIndex { 0 };
    QString tpmHashAlgo;
    QString tpmKeyAlgo;
    bool tmpAvailable { false };
};
}
#endif   //! VAULTSETUNLOCKMETHODVIEW_H
