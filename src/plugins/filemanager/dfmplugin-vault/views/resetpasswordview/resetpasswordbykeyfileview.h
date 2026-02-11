// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESETPASSWORDBYKEYFILEVIEW_H
#define RESETPASSWORDBYKEYFILEVIEW_H

#include "dfmplugin_vault_global.h"

#include <DPasswordEdit>
#include <DLabel>
#include <DFileChooserEdit>
#include <DFileDialog>
#include <DSpinner>

#include <QFrame>
#include <QFutureWatcher>

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DLabel;
class DFileChooserEdit;
class DFileDialog;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class ResetPasswordByKeyFileView : public QFrame
{
    Q_OBJECT
public:
    explicit ResetPasswordByKeyFileView(QWidget *parent = nullptr);
    ~ResetPasswordByKeyFileView() override;

    QStringList btnText();
    QString titleText();
    void buttonClicked(int index, const QString &text);

signals:
    void signalJump();  // 切换到旧密码验证
    void sigBtnEnabled(const int &index, const bool &state);
    void sigCloseDialog();

public slots:
    void onPasswordChanged();
    void onKeyFileSelected(const QString &path);

private slots:
    void onNewPasswordChanged(const QString &pwd);
    void onRepeatPasswordChanged(const QString &pwd);

private:
    void initUI();
    bool checkPassword(const QString &password);
    bool checkRepeatPassword();
    bool checkInputInfo();

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual bool eventFilter(QObject *obj, QEvent *evt) override;

private:
    DTK_WIDGET_NAMESPACE::DFileChooserEdit *keyFileEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DFileDialog *fileDialog { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *newPasswordEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *repeatPasswordEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DLineEdit *passwordHintEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *switchMethodLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DSpinner *spinner { nullptr };

    struct ResetPasswordResult {
        bool success;
    };
    QFutureWatcher<ResetPasswordResult> *resetPasswordWatcher { nullptr };

private slots:
    void onResetPasswordFinished();
};
}
#endif   // RESETPASSWORDBYKEYFILEVIEW_H

