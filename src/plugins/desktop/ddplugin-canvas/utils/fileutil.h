// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEUTIL_H
#define FILEUTIL_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

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
    FileInfoPointer createFileInfo(const QUrl &url,
                                   dfmbase::Global::CreateFileInfoType cache = dfmbase::Global::CreateFileInfoType::kCreateFileInfoAuto);

protected:
    explicit DesktopFileCreator();
};

}

#define FileCreator DDP_CANVAS_NAMESPACE::DesktopFileCreator::instance()
#endif   // FILEUTIL_H
