// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTREMOVEBYPASSWORDVIEW_H
#define VAULTREMOVEBYPASSWORDVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <QWidget>

class QLineEdit;
class QPushButton;

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DToolTip;
class DFloatingWidget;
class DLabel;
class DSpinner;
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {

class VaultRemoveByPasswordView : public QWidget
{
    Q_OBJECT
public:
    enum EN_ToolTip {
        kWarning = 0,
        kInformation
    };
    explicit VaultRemoveByPasswordView(QWidget *parent = nullptr);
    ~VaultRemoveByPasswordView() override;

    QStringList btnText() const;
    QString titleText() const;
    void buttonClicked(int index, const QString &text);

    void showAlertMessage(const QString &text, int duration = 3000);
    void showToolTip(const QString &text, int duration, EN_ToolTip enType);
    void setTipsButtonVisible(bool visible);

public Q_SLOTS:
    void onPasswordChanged(const QString &password);
    void slotCheckAuthorizationFinished(bool result);

Q_SIGNALS:
    void signalJump(const RemoveWidgetType &type);
    void sigCloseDialog();

private:
    DTK_WIDGET_NAMESPACE::DPasswordEdit *pwdEdit { nullptr };
    QPushButton *tipsBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DToolTip *tooltip { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *floatWidget { nullptr };
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *toggleModeBtn { Q_NULLPTR };
    DTK_WIDGET_NAMESPACE::DSpinner *spinner { nullptr };
};
}
#endif   // VAULTREMOVEBYPASSWORDVIEW_H
