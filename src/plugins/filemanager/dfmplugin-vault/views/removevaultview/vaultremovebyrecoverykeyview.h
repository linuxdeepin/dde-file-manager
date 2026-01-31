// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEBYRECOVERYKEYVIEW_H
#define VAULTREMOVEBYRECOVERYKEYVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QWidget>

class QPlainTextEdit;

DWIDGET_BEGIN_NAMESPACE
class DToolTip;
class DFloatingWidget;
class DSpinner;
class DFileChooserEdit;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class VaultRemoveByRecoverykeyView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveByRecoverykeyView(QWidget *parent = nullptr);
    ~VaultRemoveByRecoverykeyView() override;

    QString getRecoverykey();
    void showAlertMessage(const QString &text, int duration = 3000);

    QStringList btnText() const;
    QString titleText() const;
    void buttonClicked(int index, const QString &text);

public slots:
    void onRecoveryKeyChanged();
    void slotCheckAuthorizationFinished(bool result);

Q_SIGNALS:
    void signalJump(const RemoveWidgetType &type);
    void sigCloseDialog();

private:
    void initUI();

    //! 输入凭证后，对凭证添加“-”
    int afterRecoveryKeyChanged(QString &str);
    void checkRecoveryKeyV1();
    void checkRecoveryKeyV2();

    bool eventFilter(QObject *watched, QEvent *event) override;

    // Helper functions for recovery key validation
    bool validateRecoveryKeyV1(const QString &key);
    void handleRecoveryKeyV1ValidationResult(bool isValid);
    bool validateRecoveryKeyFile(const QString &file);
    void handleRecoveryKeyFileValidationResult(bool isValid);

private:
    QPlainTextEdit *keyEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DFileChooserEdit *filePathEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DToolTip *tooltip { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *floatWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DSpinner *spinner { nullptr };
};
}
#endif   // VAULTREMOVEBYRECOVERYKEYVIEW_H
