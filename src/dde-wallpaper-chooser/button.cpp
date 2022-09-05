// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "button.h"
#include <QKeyEvent>

Button::Button(QWidget * parent) :
    QPushButton(parent)
{
    //setFlat(true);
}

Button::~Button()
{

}

void Button::setAttract(bool attract)
{
    Q_UNUSED(attract);
//    if (attract) {
//        setStyleSheet(ATTRACT_STYLE);
//    } else {
//        setStyleSheet(NO_ATTRACT_STYLE);
//    }
}

void Button::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    //按下空格、换行、回车按钮皆发送点击信号
    case Qt::Key_Space:
    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit clicked();
        break;
//    case Qt::Key_Up:    //bug#30790屏蔽上下键
//    case Qt::Key_Down:
//        return;
    default:
    //保持按键事件传递
        event->ignore();
        break;
    }
    //QPushButton::keyPressEvent(event);
}
