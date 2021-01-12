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

#ifndef SHOWFILEPLUGIN_H
#define SHOWFILEPLUGIN_H

#include <QObject>
#include "../plugininterfaces/menu/menuinterface.h"

class QIcon;
class QAction;

#define MAXICON 5

struct cmdMenuStr
{
    int argv;     //!< 自定义参数值
    QString str;  //!< 命令值对应 的 右键的菜单的字符串
};


class ShowFilePlugin : public QObject, public MenuInterface , public PropertyDialogExpandInfoInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID MenuInterface_iid FILE "showfile.json")
    Q_INTERFACES(MenuInterface PropertyDialogExpandInfoInterface)
public:
    ShowFilePlugin(QObject *parent = 0);
    QList<QIcon> additionalIcons(const QString &file);
    QList<QAction *> additionalMenu(const QStringList &files, const QString& currentDir);
    QList<QAction *> additionalEmptyMenu(const QString& currentDir);

    QWidget* expandWidget(const QString &file);
    QString expandWidgetTitle(const QString& file);

public slots:
    void onActionTriggered();

private:
    int analyzeFile(const QString &filename, int &a, int &b );
    QIcon aIcon[ MAXICON ];     //!< 自定义格式文件中a对应的图标
    QIcon bIcon[ MAXICON ];     //!< 自定义格式文件中a对应的图标
    QStringList cmdTab;         //!< 每一个菜单的显示内容及键值，自定义值
};

typedef QList<QIcon> QIconList;
typedef QList<QAction*> QActionList;

#endif // SHOWFILEPLUGIN_H
