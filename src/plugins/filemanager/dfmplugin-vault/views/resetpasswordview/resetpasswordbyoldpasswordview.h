// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESETPASSWORDBYOLDPASSWORDVIEW_H
#define RESETPASSWORDBYOLDPASSWORDVIEW_H

#include "dfmplugin_vault_global.h"

#include <DPasswordEdit>
#include <DLabel>

#include <QFrame>

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class ResetPasswordByOldPasswordView : public QFrame
{
    Q_OBJECT
public:
    explicit ResetPasswordByOldPasswordView(QWidget *parent = nullptr);
    ~ResetPasswordByOldPasswordView() override;

    QStringList btnText();
    QString titleText();
    void buttonClicked(int index, const QString &text);

signals:
    void signalJump();  // 切换到密钥文件验证
    void sigBtnEnabled(const int &index, const bool &state);
    void sigCloseDialog();

public slots:
    void onPasswordChanged();

private slots:
    void onOldPasswordChanged(const QString &pwd);
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
    DTK_WIDGET_NAMESPACE::DPasswordEdit *oldPasswordEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *newPasswordEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DPasswordEdit *repeatPasswordEdit { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *switchMethodLabel { nullptr };
};
}
#endif   // RESETPASSWORDBYOLDPASSWORDVIEW_H

