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
#ifndef LOCALMENU_H
#define LOCALMENU_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractmenu.h"

#include <QObject>
#include <QSharedPointer>
#include <QVariant>

DFMBASE_BEGIN_NAMESPACE
class LocalMenuPrivate;
class LocalMenu : public AbstractMenu
{
    Q_OBJECT
    QSharedPointer<LocalMenuPrivate> d;

public:
    explicit LocalMenu(QObject *parent = nullptr);
    virtual QMenu *build(QWidget *parent,
                         MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &foucsUrl,
                         const QList<QUrl> &selected = {},
                         QVariant customData = QVariant());
};
DFMBASE_END_NAMESPACE

#endif   // LOCALMENU_H
