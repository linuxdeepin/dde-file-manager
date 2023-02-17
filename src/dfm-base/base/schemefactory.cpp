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
