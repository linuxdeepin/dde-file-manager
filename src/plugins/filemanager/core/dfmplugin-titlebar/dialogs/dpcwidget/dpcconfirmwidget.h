// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPCCONFIRMWIDGET_H
#define DPCCONFIRMWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <DWidget>

class QDBusInterface;

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DFloatingWidget;
class DToolTip;
class DLabel;
class DSuggestButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {

class DPCConfirmWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit DPCConfirmWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void sigCloseDialog();
    void sigConfirmed();

public Q_SLOTS:
    void checkPasswordLength(const QString &pwd);
    void onSaveBtnClicked();
    void onPasswordChecked(int result);

private:
    void initUI();
    void initConnect();
    void showToolTips(const QString &msg, QWidget *w);
    bool checkRepeatPassword();
    bool checkNewPassword();
    void setEnabled(bool enabled);

private:
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *oldPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *newPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *repeatPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *toolTipFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DToolTip *toolTip { nullptr };
    DTK_WIDGET_NAMESPACE::DSuggestButton *saveBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *cancelBtn { nullptr };
    QWidget *parentWidget { nullptr };
    QSharedPointer<QDBusInterface> accessControlInter { nullptr };
};

}

#endif   // DPCCONFIRMWIDGET_H
