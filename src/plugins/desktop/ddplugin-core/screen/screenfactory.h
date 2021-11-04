/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#ifndef SCREENFACTORY_H
#define SCREENFACTORY_H

#include "dfm_desktop_service_global.h"

#include <dfm-framework/framework.h>
#include <dfm-base/widgets/abstractscreenproxy.h>

DSB_D_BEGIN_NAMESPACE
class ScreenFactory final: dpf::QtClassFactory<dfmbase::AbstractScreenProxy>,
        dpf::QtClassManager<dfmbase::AbstractScreenProxy>
{
    Q_DISABLE_COPY(ScreenFactory)
    friend class ScreenService;
private:
    ScreenFactory();

    static ScreenFactory* instance();

    template<class T>
    static bool regClass(const QString &platform, QString *errorString = nullptr)
    {
        return ScreenFactory::instance()->
                dpf::QtClassFactory<dfmbase::AbstractScreenProxy>::regClass<T>(platform, errorString);
    }

    static dfmbase::AbstractScreenProxy* create(const QString &platform, QString *errorString = nullptr)
    {
        auto pointer = ScreenFactory::instance()->value(platform);
        if (pointer) {
            return pointer;
        }
        pointer = ScreenFactory::instance()->
                dpf::QtClassFactory<dfmbase::AbstractScreenProxy>::create(platform, errorString);
        ScreenFactory::instance()->append(platform, pointer, errorString);
        return pointer;
    }
};
DSB_D_END_NAMESPACE

#endif // SCREENFACTORY_H
