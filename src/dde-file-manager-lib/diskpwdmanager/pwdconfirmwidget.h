// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PWDCONFIRMWIDGET_H
#define PWDCONFIRMWIDGET_H

#include <DWidget>

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DFloatingWidget;
class DToolTip;
class DLabel;
class DSuggestButton;
DWIDGET_END_NAMESPACE

class DiskInterface;

class PwdConfirmWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit PwdConfirmWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void sigClosed();
    void sigConfirmed();

public Q_SLOTS:
    void checkPassword(const QString &pwd);
    void onSaveBtnClicked();
    void onPasswordChecked(bool result);

private:
    void initUI();
    void initConnect();
    void showToolTip(const QString &msg, QWidget *w);
    bool checkRepeatPassword();
    bool checkNewPassword();

private:
    DTK_WIDGET_NAMESPACE::DLabel *m_titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *m_oldPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *m_newPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *m_repeatPwdEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *m_toolTipFrame { nullptr };
    DTK_WIDGET_NAMESPACE::DToolTip *m_toolTip { nullptr };
    DTK_WIDGET_NAMESPACE::DSuggestButton *m_saveBtn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *m_cancelBtn { nullptr };
    DiskInterface *m_diskInterface { nullptr };
    QWidget *m_parentWidget { nullptr };
};

#endif   // PWDCONFIRMWIDGET_H
