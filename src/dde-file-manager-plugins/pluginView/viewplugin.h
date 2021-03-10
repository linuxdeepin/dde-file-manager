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

#ifndef VIEWPLUGIN_H
#define VIEWPLUGIN_H

#include  "../plugininterfaces/view/viewinterface.h"


class ViewPlugin : /*public QObject, */public ViewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ViewInterface_iid FILE "pluginView.json")
    Q_INTERFACES(ViewInterface)
public:
    explicit ViewPlugin(QObject *parent = nullptr);

    QString bookMarkText() override;
    QIcon bookMarkNormalIcon() override;
    QIcon bookMarkHoverIcon() override;
    QIcon bookMarkPressedIcon() override;
    QIcon bookMarkCheckedIcon() override;
    QString crumbText() override;
    QIcon crumbNormalIcon() override;
    QIcon crumbHoverIcon() override;
    QIcon crumbPressedIcon() override;
    QIcon crumbCheckedIcon() override;
    bool isAddSeparator() override;
    QString scheme() override;
    QWidget* createView() override;
};

#endif // VIEWPLUGIN_H
