/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
