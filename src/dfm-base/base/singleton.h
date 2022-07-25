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
#ifndef SINGLETION_H
#define SINGLETION_H

#include "dfm-base/dfm_base_global.h"

namespace dfmbase {

/**
 * \brief The class GC
 * 懒汉单例垃圾回收类
 */
template<class CT>
class GC
{
    CT *ins;

public:
    explicit GC(CT *instance)
    {
        ins = instance;
    }

    virtual ~GC()
    {
        if (ins) {
            delete ins;
            ins = nullptr;
        }
    }
};

/**
 * \brief The class Singletion
 * 懒汉单例
 */
template<class CT>
class Singletion
{
    static CT *ins;

public:
    explicit Singletion();
    static CT &instance()
    {
        if (!ins) {
            ins = new CT();
            static GC<CT> stack_gc(ins);
        }
        return *ins;
    }
};

}

#endif   // SINGLETION_H
