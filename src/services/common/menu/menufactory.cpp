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
#include "menufactory.h"
DSC_BEGIN_NAMESPACE

MenuFactory::MenuFactory()
{

}

MenuFactory::~MenuFactory()
{

}

AbstractFileMenu *MenuFactory::create(const QString name, QString *errorString)
{
    auto topClass = MenuFactory::instance().dpf::QtClassManager<AbstractFileMenu>::value(name);
    if (topClass)
        return topClass;
    topClass = MenuFactory::instance().dpf::QtClassFactory<AbstractFileMenu>::create(name);
    MenuFactory::instance().dpf::QtClassManager<AbstractFileMenu>::append(name, topClass, errorString); //缓存
    return topClass;
}

DSC_END_NAMESPACE
