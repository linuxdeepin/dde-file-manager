// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INVOKEHELPER_H
#define INVOKEHELPER_H

#include <dfm-framework/dfm_framework_global.h>

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
