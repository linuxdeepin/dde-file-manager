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

#include "imagemenuplugin.h"
#include "interfaces/dfilemenu.h"
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QDebug>


ImageMenuPlugin::ImageMenuPlugin(QObject *parent) :
    QObject(parent)
{

}

QList<QIcon> ImageMenuPlugin::additionalIcons(const QString &file)
{
    QList<QIcon> icons;
    QIcon icon(":/images/phone.svg");
//    icons << icon;
    return icons;
}

QList<QAction *> ImageMenuPlugin::additionalMenu(const QStringList &files, const QString& currentDir)
{
    QList<QAction *> actions;
    QAction* formatAction = new QAction(tr("Format convert"), this);
    QAction* testAction = new QAction(tr("Format test"), this);
    QAction* sublAction = new QAction(tr("SubMenu"), this);

    actions << formatAction << testAction << sublAction;


    DFileMenu* subsubmenu = new DFileMenu;
    QList<QAction *> subsublActions;
    for (int i=0; i <10;  i++) {
        QAction* action = new QAction(QString::number(i), this);
        connect(action, &QAction::triggered, [](){
            qDebug() << "=======12345=======";
        });
        subsublActions.append(action);
    }
    subsubmenu->addActions(subsublActions);


    DFileMenu* menu = new DFileMenu;
    QList<QAction *> sublActions;
    for (int i=0; i <10;  i++) {
        QAction* action = new QAction(QString::number(i), this);
        connect(action, &QAction::triggered, [=](){
            qDebug() << "======dsd========";
        });
        sublActions.append(action);

        if (i % 2 == 0){
            action->setMenu(subsubmenu);
        }
    }
    menu->addActions(sublActions);
    sublAction->setMenu(menu);

    connect(formatAction, &QAction::triggered, this, &ImageMenuPlugin::handleFormat);
    connect(testAction, &QAction::triggered, this, &ImageMenuPlugin::handleTest);

    return actions;
}

QList<QAction *> ImageMenuPlugin::additionalEmptyMenu(const QString &currentDir)
{
    QList<QAction *> actions;
    QAction* formatAction = new QAction(tr("dssdds"), this);
    QAction* testAction = new QAction(tr("sdsdds"), this);
    QAction* sublAction = new QAction(tr("sdds"), this);

    actions << formatAction << testAction << sublAction;

    return actions;
}

void ImageMenuPlugin::handleFormat()
{
    qDebug() << "1111111111111111";
}

void ImageMenuPlugin::handleTest()
{
    qDebug() << "2222222222222222";
}
