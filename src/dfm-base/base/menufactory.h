/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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
#ifndef MENUFACTORY_H
#define MENUFACTORY_H

#include "dfm-base/interfaces/abstractmenu.h"

#include <dfm-framework/service/pluginservicecontext.h>

DFMBASE_BEGIN_NAMESPACE

class Q_DECL_DEPRECATED MenuFactory final : dpf::QtClassFactory<AbstractMenu>,
                          dpf::QtClassManager<AbstractMenu>
{
    Q_DISABLE_COPY(MenuFactory)
    friend class MenuService;

    MenuFactory();
    ~MenuFactory();

public:
    template<class T>
    static bool regClass(const QString name, QString *errorString = nullptr)
    {
        return MenuFactory::instance().dpf::QtClassFactory<AbstractMenu>::regClass<T>(name, errorString);
    }

    static AbstractMenu *create(const QString name, QString *errorString = nullptr);

    static MenuFactory &instance();
};

DFMBASE_END_NAMESPACE
#endif   // MENUFACTORY_H
