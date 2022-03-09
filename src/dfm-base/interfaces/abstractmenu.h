/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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
#ifndef ABSTRACTMENU_H
#define ABSTRACTMENU_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/dfm_actiontype_defines.h"

#include <QObject>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class AbstractMenuPrivate;
class AbstractMenu : public QObject
{
    Q_OBJECT
public:
    enum MenuMode {
        kEmpty,
        kNormal,
    };

    enum MenuType {
        kSingleFile,
        kMultiFiles,
        kMultiFilesSystemPathIncluded,
        kSpaceArea
    };
    explicit AbstractMenu(QObject *parent = nullptr);
    virtual ~AbstractMenu();
    virtual QMenu *build(QWidget *parent,
                         MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &focusUrl,
                         const QList<QUrl> &selected = {},
                         QVariant customData = QVariant());
    virtual void actionBusiness(QAction *act);
};

DFMBASE_END_NAMESPACE

#endif   // ABSTRACTMENU_H
