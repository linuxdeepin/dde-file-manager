// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractbackground.h"

namespace dfmbase {

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

}
