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
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {

class VaultRemoveByPasswordView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultRemoveByPasswordView(QWidget *parent = nullptr);
    ~VaultRemoveByPasswordView() override;
    enum EN_ToolTip {
        kWarning = 0,
        kInformation
    };

    /*!
    * /brief    获取密码
    */
    QString getPassword();

    /*!
    * /brief    清空密码
    */
    void clear();

    /*!
     * /brief showAlertMessage 显示密码输入框提示信息
     * /param text  信息内容
     * /param duration  显示时长
     */
    void showAlertMessage(const QString &text, int duration = 3000);
    void showToolTip(const QString &text, int duration, EN_ToolTip enType);

    /*!
     * /brief setTipsButtonVisible 设置提示按钮是否可见
     * /param visible
     */
    void setTipsButtonVisible(bool visible);
public slots:
    void onPasswordChanged(const QString &password);

private:
    DTK_WIDGET_NAMESPACE::DPasswordEdit *pwdEdit { nullptr };
    QPushButton *tipsBtn { nullptr };

    DTK_WIDGET_NAMESPACE::DToolTip *tooltip { nullptr };
    DTK_WIDGET_NAMESPACE::DFloatingWidget *floatWidget { nullptr };
};
}
#endif   // VAULTREMOVEBYPASSWORDVIEW_H
