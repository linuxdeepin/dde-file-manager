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
#include "localfilemenu.h"
#include "dfm-base/base/urlroute.h"

#include <QUrl>
#include <QDir>

DFMBASE_BEGIN_NAMESPACE

LocalFileMenu::LocalFileMenu(QObject *parent)
    : AbstractFileMenu(parent)
{
}

QMenu *LocalFileMenu::build(AbstractFileMenu::MenuMode mode,
                            const QUrl &rootUrl, const QUrl &foucsUrl,
                            const QList<QUrl> &selected)
{
    QString path = UrlRoute::urlToPath(rootUrl);
    if (!QDir(path).exists())
        return nullptr;
    return AbstractFileMenu::build(mode, rootUrl, foucsUrl, selected);
}

DFMBASE_END_NAMESPACE
