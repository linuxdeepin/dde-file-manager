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
#ifndef BACKGROUNDFACTORY_H
#define BACKGROUNDFACTORY_H

#include "dfm_desktop_service_global.h"
#include "dfm-base/widgets/abstractbackground.h"

#include <dfm-framework/framework.h>

DSB_D_BEGIN_NAMESPACE

class BackgroundFactory final: dpf::QtClassFactory<dfmbase::AbstractBackground>,
        dpf::QtClassManager<dfmbase::AbstractBackground>
{
    friend class BackgroundService;
    BackgroundFactory();
    static BackgroundFactory* instance()
    {
        static BackgroundFactory factory;
        return &factory;
    }

    template<class T>
    static bool regClass(const QString &screenName, QString *errorString = nullptr)
    {
        return BackgroundFactory::instance()->
                dpf::QtClassFactory<dfmbase::AbstractBackground>::regClass<T>(screenName, errorString);
    }

    static dfmbase::AbstractBackground* create(const QString &screenName, QString *errorString = nullptr)
    {
        auto pointer = BackgroundFactory::instance()->value(screenName);
        if (pointer) {
            return pointer;
        }
        pointer = BackgroundFactory::instance()->
                dpf::QtClassFactory<dfmbase::AbstractBackground>::create(screenName, errorString);
        BackgroundFactory::instance()->append(screenName, pointer, errorString);
        return pointer;
    }
};
DSB_D_END_NAMESPACE

#endif // BACKGROUNDFACTORY_H
