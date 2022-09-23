// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>
#include <QLabel>

class Button : public QPushButton
{
    Q_OBJECT
public:
    explicit Button(QWidget * parent = nullptr);
    ~Button();

    void setAttract(bool);

protected:
    void keyPressEvent(QKeyEvent *event);
};

class EditLabel :public QLabel
{
    Q_OBJECT
public:
    explicit EditLabel(QWidget *parent = nullptr);

private:
    void mousePressEvent(QMouseEvent *event) override;

Q_SIGNALS:
    void editLabelClicked();

};

#endif // BUTTON_H
