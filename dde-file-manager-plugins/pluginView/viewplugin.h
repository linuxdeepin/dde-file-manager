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

#ifndef VIEWPLUGIN_H
#define VIEWPLUGIN_H

#include  "../plugininterfaces/view/viewinterface.h"


class ViewPlugin : public QObject, public ViewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ViewInterface_iid FILE "pluginView.json")
    Q_INTERFACES(ViewInterface)
public:
    ViewPlugin(QObject *parent = 0);

    QString bookMarkText();
    QIcon bookMarkNormalIcon();
    QIcon bookMarkHoverIcon();
    QIcon bookMarkPressedIcon();
    QIcon bookMarkCheckedIcon();
    QString crumbText();
    QIcon crumbNormalIcon();
    QIcon crumbHoverIcon();
    QIcon crumbPressedIcon();
    QIcon crumbCheckedIcon();
    bool isAddSeparator();
    QString scheme();
    QWidget* createView();
};

#endif // VIEWPLUGIN_H
