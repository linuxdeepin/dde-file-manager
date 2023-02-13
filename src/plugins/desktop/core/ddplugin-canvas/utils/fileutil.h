// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEUTIL_H
#define FILEUTIL_H

#include "ddplugin_canvas_global.h"

#include <base/urlroute.h>
#include <file/local/localfileinfo.h>

#include <QUrl>

namespace ddplugin_canvas {
#if 0   // destop should use file:// rather than desktop://.
inline QUrl covertDesktopUrlToFile(const QUrl &desktopUrl) {
    QUrl fileUrl = DFMBASE_NAMESPACE::UrlRoute::pathToUrl(
                DFMBASE_NAMESPACE::UrlRoute::urlToPath(desktopUrl),
                DFMBASE_NAMESPACE::Global::Scheme::kFile);
    return fileUrl;
}

static inline QUrl covertFileUrlToDesktop(const QUrl &fileUrl){
    QUrl desktopUrl = DFMBASE_NAMESPACE::UrlRoute::pathToReal(
                DFMBASE_NAMESPACE::UrlRoute::urlToPath(fileUrl));
    return desktopUrl;
}
#endif

class DesktopFileCreator
{
public:
    static DesktopFileCreator *instance();
    DFMLocalFileInfoPointer createFileInfo(const QUrl &url, bool cache = true);

protected:
    explicit DesktopFileCreator();
};

}

#define FileCreator DDP_CANVAS_NAMESPACE::DesktopFileCreator::instance()
#endif   // FILEUTIL_H
