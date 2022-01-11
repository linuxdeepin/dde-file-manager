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
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/urlroute.h"

#include <QUrl>
#include <QDomDocument>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>

DPRECENT_BEGIN_NAMESPACE
class RecentUtil final
{

public:
    static QString sidebarDisplayText;

    explicit RecentUtil() = delete;

    inline static QDomNodeList getRecentNodes()
    {
        return nodes;
    }

    static bool initRecentSubSystem();

    inline static QString scheme()
    {
        return "recent";
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("document-open-recent-symbolic");
    }

    static QUrl rootUrl()
    {
        QUrl url;
        url.setScheme(scheme());
        url.setPath("/");
        return url;
    }

private:
    static QDomNodeList nodes;
};
DPRECENT_END_NAMESPACE
#endif   // DFMRECENTUTIL_H
