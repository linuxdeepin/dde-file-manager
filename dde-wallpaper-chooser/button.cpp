/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#define NO_ATTRACT_STYLE "QPushButton { color: white; border-width: 6 6 10 6 }" \
    "QPushButton { border-image: url(':/images/transparent_button_normal.svg') 6 6 10 6 }" \
    "QPushButton:hover { border-image: url(':/images/transparent_button_hover.svg') 6 6 10 6 }" \
    "QPushButton:pressed { border-image: url(':/images/transparent_button_press.svg') 6 6 10 6 }"

#define ATTRACT_STYLE "QPushButton { color: white; border-width: 6 6 10 6 }" \
    "QPushButton { border-image: url(':/images/button_normal.svg') 6 6 10 6 }" \
    "QPushButton:hover { border-image: url(':/images/button_hover.svg') 6 6 10 6 }" \
    "QPushButton:pressed { border-image: url(':/images/button_press.svg') 6 6 10 6 }"

Button::Button(QWidget * parent) :
    QPushButton(parent)
{
    setFlat(true);
    setFixedSize(160, 36);
}

Button::~Button()
{

}

void Button::setAttract(bool attract)
{
    if (attract) {
        setStyleSheet(ATTRACT_STYLE);
    } else {
        setStyleSheet(NO_ATTRACT_STYLE);
    }
}
