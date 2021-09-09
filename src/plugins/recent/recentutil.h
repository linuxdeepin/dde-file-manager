/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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

#include "recentlog.h"

#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/urlroute.h"

#include <QUrl>
#include <QDomDocument>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>

const char RECENT_SCHEME[] = "recent";


///home/funning/.local/share/recently-used.xbel

class RecentUtil final
{
    inline static QDomDocument recentDom;
    inline static QDomNodeList nodes;
    inline static QFile recentFile; // 二级表树
public:

    RecentUtil() = delete;

    static bool initRecentSubSystem()
    {
        qCCritical(RecentPlugin) << QStandardPaths::locate(QStandardPaths::HomeLocation,"",QStandardPaths::LocateDirectory);

        recentFile.setFileName(QStandardPaths::locate(QStandardPaths::HomeLocation,"",QStandardPaths::LocateDirectory)
                + ".local/share/recently-used.xbel");

        if (!recentFile.open(QFile::OpenModeFlag::ReadOnly)) {
            qCCritical(RecentPlugin) << "Failed, open recent file:" << recentFile.fileName();
            return false;
        }

        if (!recentDom.setContent(&recentFile)) {
            qCCritical(RecentPlugin) << "Failed, QDomDocument setContent recent file";
            return false;
        }

        nodes = recentDom.elementsByTagName("bookmark");

        return true;
    }

    static QUrl onlyRootUrl()
    {
        QUrl url;
        url.setHost(" ");
        url.setScheme("recent");
        url.setPath("/");
        return url;
    }

};

#endif // DFMRECENTUTIL_H
