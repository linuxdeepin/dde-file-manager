// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
