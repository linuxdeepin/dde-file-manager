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

#ifndef IMAGEMENUPLUGIN_H
#define IMAGEMENUPLUGIN_H

#include <QObject>
#include "../plugininterfaces/menu/menuinterface.h"


class ImageMenuPlugin : public QObject, public MenuInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID MenuInterface_iid FILE "ImageMenuPlugin.json")
    Q_INTERFACES(MenuInterface)

public:
    ImageMenuPlugin(QObject *parent = 0);

    QList<QIcon> additionalIcons(const QString &file);
    QList<QAction *> additionalMenu(const QStringList &files, const QString& currentDir);
    QList<QAction*> additionalEmptyMenu(const QString& currentDir);

public:
    void handleFormat();
    void handleTest();
};

#endif // IMAGEMENUPLUGIN_H
