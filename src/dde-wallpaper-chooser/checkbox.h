// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QCheckBox>

class CheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit CheckBox(QWidget *parent = nullptr);
    explicit CheckBox(const QString &text, QWidget *parent = nullptr);
    ~CheckBox() override {}

    /**
     * @brief keyPressEvent 重写，实现回车勾选操作
     * @param event 键盘事件
     */
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // CHECKBOX_H
