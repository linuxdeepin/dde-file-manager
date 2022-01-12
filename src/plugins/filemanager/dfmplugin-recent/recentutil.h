/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMRECENTUTIL_H
#define DFMRECENTUTIL_H

#include "dfmplugin_recent_global.h"

#include <QUrl>
#include <QDebug>
#include <QDir>
#include <QIcon>

DPRECENT_BEGIN_NAMESPACE
class RecentUtil final
{

public:
    inline static QString scheme()
    {
        return "recent";
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("document-open-recent-symbolic");
    }

    inline static QString xbelPath()
    {
        return QDir::homePath() + "/.local/share/recently-used.xbel";
    }

    static QUrl rootUrl()
    {
        QUrl url;
        url.setScheme(scheme());
        url.setPath("/");
        return url;
    }

private:
    explicit RecentUtil() = delete;
};
DPRECENT_END_NAMESPACE
#endif   // DFMRECENTUTIL_H
