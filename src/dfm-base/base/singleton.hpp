/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef SINGLETON_H
#define SINGLETON_H

#include "dfm-base/dfm_base_global.h"

DFMBASE_BEGIN_NAMESPACE
template <class T>
class Singleton
{
public:
    static inline T *instance()
    {
        static T _instance;
        return &_instance;
    }

protected:
    Singleton(void) {}
    ~Singleton(void) {}
    Singleton(const Singleton &) {}
    Singleton &operator= (const Singleton &);
};

DFMBASE_END_NAMESPACE

#define GOLOBAL_IMPORT_SINGLETON(x) \
    static x* _ins = DFMBASE_NAMESPACE::Singleton<x>::instance();

#endif // SINGLETON_H
