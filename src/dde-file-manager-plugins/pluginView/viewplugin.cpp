/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#include "viewplugin.h"
#include <QLabel>

ViewPlugin::ViewPlugin(QObject *parent) :
    ViewInterface(parent)
{
    Q_INIT_RESOURCE(pluginview);
}

QString ViewPlugin::bookMarkText()
{
    return "view";
}

QIcon ViewPlugin::bookMarkNormalIcon()
{
    return QIcon(":/images/release.png");
}

QIcon ViewPlugin::bookMarkHoverIcon()
{
    return QIcon(":/images/hover.png");
}

QIcon ViewPlugin::bookMarkPressedIcon()
{
    return QIcon(":/images/pressed.svg");
}

QIcon ViewPlugin::bookMarkCheckedIcon()
{
    return QIcon(":/images/checked.svg");
}

QString ViewPlugin::crumbText()
{
    return "view";
}

QIcon ViewPlugin::crumbNormalIcon()
{
    return QIcon(":/images/release.png");
}

QIcon ViewPlugin::crumbHoverIcon()
{
    return QIcon(":/images/hover.png");
}

QIcon ViewPlugin::crumbPressedIcon()
{
    return QIcon(":/images/hover.png");
}

QIcon ViewPlugin::crumbCheckedIcon()
{
    return QIcon(":/images/checked.svg");
}

bool ViewPlugin::isAddSeparator()
{
    return true;
}

QString ViewPlugin::scheme()
{
    return "view";
}

QWidget *ViewPlugin::createView()
{
    QLabel* label = new QLabel;
    label->setText("view");
    label->setAlignment(Qt::AlignCenter);
//    label->setStyleSheet("background-color: green; margins:0");
    return label;
}
