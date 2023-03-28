// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "schemefactory.h"
#include "dfm-base/base/singleton.h"

namespace dfmbase {

InfoFactory *InfoFactory::ins { nullptr };
ViewFactory *ViewFactory::ins { nullptr };
WatcherFactory *WatcherFactory::ins { nullptr };
DirIteratorFactory *DirIteratorFactory::ins { nullptr };
SortAndFitersFactory *SortAndFitersFactory::ins { nullptr };

InfoFactory &InfoFactory::instance()
{
    if (!ins) {
        ins = new InfoFactory();
        static GC<InfoFactory> gc(ins);
    }
    return *ins;
}

QString InfoFactory::scheme(const QUrl &url)
{
    auto scheme = url.scheme();
    if (scheme == Global::Scheme::kFile && !FileUtils::isLocalDevice(url))
        return Global::Scheme::kAsyncFile;

    dfmio::DFileInfo dinfo(url);
    auto targetPath = dinfo.attribute(dfmio::DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
    if (!targetPath.isEmpty() && !FileUtils::isLocalDevice(QUrl::fromLocalFile(targetPath)))
        scheme = Global::Scheme::kAsyncFile;

    return scheme;
}

WatcherFactory &WatcherFactory::instance()
{
    if (!ins) {
        ins = new WatcherFactory();
        static GC<WatcherFactory> gc(ins);
    }
    return *ins;
}

DirIteratorFactory &DirIteratorFactory::instance()
{
    if (!ins) {
        ins = new DirIteratorFactory();
        static GC<DirIteratorFactory> gc(ins);
    }
    return *ins;
}

ViewFactory &ViewFactory::instance()
{
    if (!ins) {
        ins = new ViewFactory();
        static GC<ViewFactory> gc(ins);
    }
    return *ins;
}

SortAndFitersFactory &SortAndFitersFactory::instance()
{
    if (!ins) {
        ins = new SortAndFitersFactory();
        static GC<SortAndFitersFactory> gc(ins);
    }
    return *ins;
}

}
