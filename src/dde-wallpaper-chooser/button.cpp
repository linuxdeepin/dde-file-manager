/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             xinglinkun<xinglinkun@uniontech.com>
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
