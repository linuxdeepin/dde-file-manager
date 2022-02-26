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
#include "abstractmenu.h"

#include <QAction>
#include <QMenu>
#include <QDebug>

DFMBASE_BEGIN_NAMESPACE

AbstractMenu::AbstractMenu(QObject *parent)
    : QObject(parent)
{
}

AbstractMenu::~AbstractMenu()
{
}

QMenu *AbstractMenu::build(QWidget *parent,
                           AbstractMenu::MenuMode mode,
                           const QUrl &rootUrl,
                           const QUrl &foucsUrl,
                           const QList<QUrl> &selected,
                           QVariant customData)
{
    Q_UNUSED(parent)
    Q_UNUSED(mode)
    Q_UNUSED(rootUrl)
    Q_UNUSED(foucsUrl)
    Q_UNUSED(selected)
    Q_UNUSED(customData)

    return nullptr;
}

void AbstractMenu::acitonBusiness(QAction *act)
{
    Q_UNUSED(act)
    // TODO :default action business
    qDebug() << "default action business: " << act->data().toString();
}

DFMBASE_END_NAMESPACE
