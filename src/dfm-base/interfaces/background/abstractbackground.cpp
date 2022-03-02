/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "abstractbackground.h"

DFMBASE_BEGIN_NAMESPACE

AbstractBackground::AbstractBackground(const QString &screenName, QWidget *parent)
    : QWidget(parent)
    , screen(screenName)
{

}

void AbstractBackground::setMode(int mode)
{
    displayMode = static_cast<Mode>(mode);
}

int AbstractBackground::mode() const
{
    return displayMode;
}

void AbstractBackground::setZoom(int zoom)
{
    displayZoom = static_cast<Mode>(zoom);
}

int AbstractBackground::zoom() const
{
    return displayZoom;
}

void AbstractBackground::setDisplay(const QString &path)
{
    filePath = path;
}

QString AbstractBackground::display() const
{
    return filePath;
}

void AbstractBackground::updateDisplay()
{

}

DFMBASE_END_NAMESPACE
