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

#ifndef DADVANCEDINFOPLUGIN_H
#define DADVANCEDINFOPLUGIN_H

#include <../dde-file-manager-plugins/plugininterfaces/menu/menuinterface.h>
#include <QObject>

class QWidget;

class DAdvancedInfoPlugin : public PropertyDialogExpandInfoInterface
{
    Q_OBJECT
    Q_INTERFACES(PropertyDialogExpandInfoInterface)
    Q_PLUGIN_METADATA(IID PropertyDialogExpandInfoInterface_iid FILE "dde-advanced-property-plugin.json")

public:
    explicit DAdvancedInfoPlugin(QObject *parent = nullptr);

    QWidget* expandWidget(const QString &file) override;

    QString expandWidgetTitle(const QString &file) override;
};

#endif // DADVANCEDINFOPLUGIN_H
