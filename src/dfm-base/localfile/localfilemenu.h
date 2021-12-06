/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef LOCALFILEMENU_H
#define LOCALFILEMENU_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/base/abstractfilemenu.h"

#include <QObject>
#include <QSharedPointer>

DFMBASE_BEGIN_NAMESPACE
class LocalMenuPrivate;
class LocalFileMenu : public AbstractFileMenu
{
    Q_OBJECT
    QSharedPointer<LocalMenuPrivate> d;

public:
    explicit LocalFileMenu(QObject *parent = nullptr);
    virtual QMenu *build(MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &foucsUrl,
                         const QList<QUrl> &selected = {});
};
DFMBASE_END_NAMESPACE

#endif   // LOCALFILEMENU_H
