/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "schemefactory.h"
#include "dfm-base/base/singleton.h"

namespace dfmbase {

InfoFactory *InfoFactory::ins { nullptr };
ViewFactory *ViewFactory::ins { nullptr };
WatcherFactory *WatcherFactory::ins { nullptr };
DirIteratorFactory *DirIteratorFactory::ins { nullptr };

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

}
