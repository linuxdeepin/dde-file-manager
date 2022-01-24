/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef DFMRECENTHELPER_H
#define DFMRECENTHELPER_H

#include "dfmplugin_recent_global.h"

#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QUrl>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QFile>

DPRECENT_BEGIN_NAMESPACE

class RecentHelper final
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

    static QUrl rootUrl();

    static void clearRecent();
    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static bool openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error);

    static bool writeToClipBoardHandle(const quint64 windowId,
                                       const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                       const QList<QUrl> urls);

private:
    explicit RecentHelper() = delete;
};
DPRECENT_END_NAMESPACE
#endif   // DFMRECENTHELPER_H
