/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "modeldatahandler.h"

#include <QUrl>

using namespace ddplugin_organizer;

ModelDataHandler::ModelDataHandler()
{

}

ModelDataHandler::~ModelDataHandler()
{

}

bool ModelDataHandler::acceptInsert(const QUrl &url)
{
    Q_UNUSED(url)
    return true;
}

bool ModelDataHandler::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(oldUrl)
    Q_UNUSED(newUrl)
    return true;
}

QList<QUrl> ModelDataHandler::acceptReset(const QList<QUrl> &urls)
{
    return urls;
}
