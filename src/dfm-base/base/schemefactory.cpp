// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/fileutils.h>

namespace dfmbase {

InfoFactory &InfoFactory::instance()
{
    static InfoFactory ins;
    return ins;
}

QString InfoFactory::scheme(const QUrl &url)
{
    auto scheme = url.scheme();
    if (scheme != Global::Scheme::kFile)
        return scheme;

    if (ProtocolUtils::isRemoteFile(url))
        return Global::Scheme::kAsyncFile;

    // Use readlink (non-blocking) to check symlink target; creating DFileInfo triggers
    // statx via g_file_query_info which follows the symlink and blocks on stale SMB mounts
    auto targetPath = FileUtils::symlinkTarget(url);
    if (!targetPath.isEmpty() && ProtocolUtils::isRemoteFile(QUrl::fromLocalFile(targetPath)))
        scheme = Global::Scheme::kAsyncFile;

    return scheme;
}

QSharedPointer<FileInfo> InfoFactory::getFileInfoFromCache(const QUrl &url, Global::CreateFileInfoType type, QString *errorString)
{
    QSharedPointer<FileInfo> info = InfoCacheController::instance().getCacheInfo(url);
    if (!info) {
        if (type == Global::CreateFileInfoType::kCreateFileInfoSyncAndCache) {
            info = instance().SchemeFactory<FileInfo>::create(url, errorString);
        } else if (type == Global::CreateFileInfoType::kCreateFileInfoAsyncAndCache) {
            info = instance().SchemeFactory<FileInfo>::create(Global::Scheme::kAsyncFile, url, errorString);
            if (info) {
                info->refresh();
            }
        }
        if (info)
            emit InfoCacheController::instance().cacheFileInfo(url, info);
    }
    return info;
}

WatcherFactory &WatcherFactory::instance()
{
    static WatcherFactory ins;
    return ins;
}

DirIteratorFactory &DirIteratorFactory::instance()
{
    static DirIteratorFactory ins;
    return ins;
}

ViewFactory &ViewFactory::instance()
{
    static ViewFactory ins;
    return ins;
}

SortFilterFactory &SortFilterFactory::instance()
{
    static SortFilterFactory ins;
    return ins;
}

}   // namespace dfmbase
