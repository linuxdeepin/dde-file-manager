/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef ABSTRACTFILEMENU_H
#define ABSTRACTFILEMENU_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
class AbstractFileMenuPrivate;

namespace FileMenuTypes {
extern const QString CREATE_NEW_DIR;
extern const QString CREATE_NEW_DOC;
extern const QString VIEW_MODE_SWITCH;
extern const QString VIEW_SORT_SWITCH;
extern const QString OPEN_AS_ADMIN;
extern const QString OPEN_IN_TERMINAL;
extern const QString SELECT_ALL;
extern const QString PROPERTY;
extern const QString CUT;
extern const QString COPY;
extern const QString PASTE;
extern const QString RENAME;
} // namesapce FileMenuTypes

class AbstractFileMenu : public QObject
{
    Q_OBJECT
public:
    enum MenuMode{
        Empty,
        Normal,
    };

    explicit AbstractFileMenu(QObject *parent = nullptr);
    virtual ~AbstractFileMenu();
    virtual QMenu *build(MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &foucsUrl,
                         const QList<QUrl> &selected = {});
};

DFMBASE_END_NAMESPACE

#endif // ABSTRACTFILEMENU_H
