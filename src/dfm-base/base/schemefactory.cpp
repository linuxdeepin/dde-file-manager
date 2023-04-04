// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/schemefactory.h>
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
    if (scheme == Global::Scheme::kFile && !FileUtils::isLocalDevice(url))
        return Global::Scheme::kAsyncFile;

    dfmio::DFileInfo dinfo(url);
    if (!dinfo.attribute(dfmio::DFileInfo::AttributeID::kStandardIsSymlink).toBool())
        return scheme;

    auto targetPath = dinfo.attribute(dfmio::DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
    if (!targetPath.isEmpty() && !FileUtils::isLocalDevice(QUrl::fromLocalFile(targetPath)))
        scheme = Global::Scheme::kAsyncFile;

    return scheme;
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
