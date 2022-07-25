/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
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
#include "sharefilehelper.h"
#include "events/shareeventscaller.h"

#include <QUrl>

DPMYSHARES_USE_NAMESPACE
ShareFileHelper *ShareFileHelper::instance()
{
    static ShareFileHelper ins;
    return &ins;
}

ShareFileHelper::ShareFileHelper(QObject *parent)
    : QObject(parent)
{
}

bool ShareFileHelper::openFileInPlugin(quint64 windowId, const QList<QUrl> urls)
{
    if (urls.isEmpty())
        return false;
    if (urls.first().scheme() != scheme())
        return false;

    ShareEventsCaller::sendOpenDirs(windowId, urls, ShareEventsCaller::OpenMode::kOpenInCurrentWindow);
    return true;
}
