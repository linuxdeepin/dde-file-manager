// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCRYPTIONPAGE_H
#define ENCRYPTIONPAGE_H

#include "preview_plugin_global.h"

#include <QWidget>
#include <DPushButton>
#include <DLabel>
#include <DPasswordEdit>

namespace plugin_filepreview {
class EncryptionPage : public QWidget
{
    Q_OBJECT
public:
    explicit EncryptionPage(QWidget *parent = nullptr);
    ~EncryptionPage();

    /**
     * @brief InitUI
     * 初始化控件
     */
    void InitUI();

    /**
     * @brief InitConnection
     * 初始化connect
     */
    void InitConnection();

signals:
    /**
     * @brief sigExtractPassword
     * 密码输入确认信号
     * @param password
     */
    void sigExtractPassword(const QString &password);

public slots:
    /**
     * @brief nextbuttonClicked
     * 密码输入确认
     */
    void nextbuttonClicked();

    /**
     * @brief wrongPassWordSlot
     * 密码错误响应
     */
    void wrongPassWordSlot();

    /**
     * @brief onPasswordChanged
     * 密码输入框字符改变
     */
    void onPasswordChanged();

    /**
     * @brief onSetPasswdFocus
     * 设置密码输入框的焦点,在切换到当前窗口时
     */
    void onSetPasswdFocus();

private slots:
    /**
     * @brief onUpdateTheme
     * 主题更新
     */
    void onUpdateTheme();

private:
    DTK_WIDGET_NAMESPACE::DPushButton *nextbutton = nullptr;
    DTK_WIDGET_NAMESPACE::DPushButton *echoBtn = nullptr;
    DTK_WIDGET_NAMESPACE::DPasswordEdit *passwordEdit = nullptr;
};
}
#endif   // ENCRYPTIONPAGE_H
