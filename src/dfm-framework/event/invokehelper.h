/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef INVOKEHELPER_H
#define INVOKEHELPER_H

#include "dfm-framework/dfm_framework_global.h"

#include <QVariantList>

DPF_BEGIN_NAMESPACE

inline void packParamsHelper(QVariantList &ret)
{
    Q_UNUSED(ret);
}

template<typename Arg>
void packParamsHelper(QVariantList &ret, Arg &&arg)
{
    ret << QVariant::fromValue(arg);
}
// QVariant::fromValue compile failes with [ Arg = char* ]
inline void packParamsHelper(QVariantList &ret, char *arg)
{
    ret << QVariant::fromValue(QString::fromUtf8(arg));
}
inline void packParamsHelper(QVariantList &ret, const char *arg)
{
    ret << QVariant::fromValue(QString::fromUtf8(arg));
}

template<typename Arg, typename... Args>
void packParamsHelper(QVariantList &ret, Arg &&arg, Args &&... args)
{
    ret << QVariant::fromValue(arg);
    packParamsHelper(ret, std::forward<Args>(args)...);
}
// QVariant::fromValue compile failes with [ Arg = char* ]
template<typename... Args>
void packParamsHelper(QVariantList &ret, char *arg, Args &&... args)
{
    ret << QVariant::fromValue(QString::fromUtf8(arg));
    packParamsHelper(ret, std::forward<Args>(args)...);
}
template<typename... Args>
void packParamsHelper(QVariantList &ret, const char *arg, Args &&... args)
{
    ret << QVariant::fromValue(QString::fromUtf8(arg));
    packParamsHelper(ret, std::forward<Args>(args)...);
}

template<class T, class... Args>
inline void makeVariantList(QVariantList *list, T t, Args &&... args)
{
    *list << QVariant::fromValue(t);
    if (sizeof...(args) > 0)
        packParamsHelper(*list, std::forward<Args>(args)...);
}

DPF_END_NAMESPACE

#endif   // INVOKEHELPER_H
