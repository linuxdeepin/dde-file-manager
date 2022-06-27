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
#ifndef AVFSUTILS_H
#define AVFSUTILS_H

#include "dfmplugin_avfsbrowser_global.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "dfm-base/mimetype/mimetypedisplaymanager.h"

#include <QString>
#include <QUrl>

namespace dfmplugin_avfsbrowser {

class AvfsUtils
{
public:
    static inline QString scheme()
    {
        return "avfs";
    }

    static inline QUrl rootUrl()
    {
        QUrl u;
        u.setScheme(scheme());
        u.setPath("/");
        return u;
    }

    static inline QStringList supportedArchives()
    {
        return DFMBASE_NAMESPACE::MimeTypeDisplayManager::supportArchiveMimetypes();
    }
    static bool isSupportedArchives(const QUrl &url);
    static bool isSupportedArchives(const QString &path);

    static bool isAvfsMounted();
    static void mountAvfs();
    static void unmountAvfs();
    static QString avfsMountPoint();
    static bool archivePreviewEnabled();

    static QUrl avfsUrlToLocal(const QUrl &avfsUrl);
    static QUrl localUrlToAvfsUrl(const QUrl &url);
    static QUrl localArchiveToAvfsUrl(const QUrl &url);
    static QUrl makeAvfsUrl(const QString &path);

    static QList<DSB_FM_NAMESPACE::TitleBar::CrumbData> seperateUrl(const QUrl &url);
    static QString parseDirIcon(const QString &path);
};

}

#endif   // AVFSUTILS_H
