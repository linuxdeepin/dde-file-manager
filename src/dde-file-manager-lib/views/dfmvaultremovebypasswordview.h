// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMVAULTREMOVEBYPASSWORDVIEW_H
#define DFMVAULTREMOVEBYPASSWORDVIEW_H

#include <dtkwidget_global.h>

#include <QWidget>

class QLineEdit;
class QPushButton;

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DToolTip;
class DFloatingWidget;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class DFMVaultRemoveByPasswordView : public QWidget
{
    Q_OBJECT
public:
    explicit DFMVaultRemoveByPasswordView(QWidget *parent = nullptr);
    ~DFMVaultRemoveByPasswordView() override;
    enum EN_ToolTip {
        Warning = 0,
        Information
    };

    /**
    * @brief    获取密码
    */
    QString getPassword();

    /**
    * @brief    清空密码
    */
    void clear();

    /**
     * @brief showAlertMessage 显示密码输入框提示信息
     * @param text  信息内容
     * @param duration  显示时长
     */
    void showAlertMessage(const QString &text, int duration = 3000);
    void showToolTip(const QString &text, int duration, EN_ToolTip enType);

    /**
     * @brief setTipsButtonVisible 设置提示按钮是否可见
     * @param visible
     */
    void setTipsButtonVisible(bool visible);
public slots:
    void onPasswordChanged(const QString &password);

private:
    DPasswordEdit *m_pwdEdit {nullptr};
    QPushButton *m_tipsBtn {nullptr};

    DToolTip *m_tooltip {nullptr};
    DFloatingWidget *m_frame {nullptr};
};

#endif // DFMVAULTREMOVEBYPASSWORDVIEW_H
