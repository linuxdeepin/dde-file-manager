/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "shareutils.h"

#include "dfm-base/base/urlroute.h"

#include <QDebug>

DPSHARES_USE_NAMESPACE

QString ShareUtils::scheme()
{
    return "usershare";
}

QIcon ShareUtils::icon()
{
    return QIcon::fromTheme("folder-publicshare");
}

QString ShareUtils::displayName()
{
    return QObject::tr("My Shares");
}

QUrl ShareUtils::rootUrl()
{
    QUrl u;
    u.setScheme(scheme());
    u.setPath("/");
    return u;
}

QUrl ShareUtils::makeShareUrl(const QString &path)
{
    QUrl u;
    u.setScheme(scheme());
    u.setPath(path);
    return u;
}

QUrl ShareUtils::convertToLocalUrl(const QUrl &shareUrl)
{
    if (shareUrl.scheme() != scheme())
        return {};
    QUrl u = shareUrl;
    u.setScheme(dfmbase::SchemeTypes::kFile);
    return u;
}

bool ShareUtils::openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error)
{
    qDebug() << "Hello";
    return true;
}
