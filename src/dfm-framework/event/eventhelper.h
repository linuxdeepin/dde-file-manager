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
#ifndef EVENTHELPER_H
#define EVENTHELPER_H

#include "dfm-framework/dfm_framework_global.h"

#include <QDebug>
#include <QVariant>
#include <QObject>

DPF_BEGIN_NAMESPACE

using EventType = int;

enum EventTypeScope : EventType {
    // default invalid id
    kInValid = -1,

    // scope of well konwn id
    kWellKnownEventBase = 0,
    kWellKnownEventTop = 10000,

    // scope of custom id
    kCustomBase = kWellKnownEventTop + 1,
    kCustomTop = 65535
};

inline EventType genCustomEventId()
{
    static EventType id = EventTypeScope::kCustomBase;
    if (id > EventTypeScope::kCustomTop)
        id = EventTypeScope::kInValid;
    return id++;
}

inline bool isValidEventType(EventType type)
{
    return type > EventTypeScope::kInValid && type <= EventTypeScope::kCustomTop;
}

/*
 * Check return value type
 */
template<typename T>
struct ReturnTypeHelper;

template<typename R, typename... Args>
struct ReturnTypeHelper<R(Args...)>
{
    using type = R;
};

template<typename R, typename... Args>
struct ReturnTypeHelper<R (*)(Args...)>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...)>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) &>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) &&>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const &>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const &&>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) volatile>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) volatile &>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) volatile &&>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const volatile>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const volatile &>
{
    using type = R;
};

template<typename R, typename C, typename... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const volatile &&>
{
    using type = R;
};

template<typename T>
using ReturnType = typename ReturnTypeHelper<T>::type;

/*
 * copy from qobjectdefs_impl.h
 * trick to set the return value of a slot that works even if the signal or the slot returns void
 * to be used like     function(), ApplyReturnValue<ReturnType>(ret.data()urn_value)
 * if function() returns a value, the operator,(T, ApplyReturnValue<ReturnType>) is called, but if it
 * returns void, the builtin one is used without an error.
 */
template<class T>
struct ApplyReturnValue
{
    void *data;
    explicit ApplyReturnValue(void *data_)
        : data(data_) {}
};
template<class T, class U>
void operator,(const T &value, const ApplyReturnValue<U> &container)
{
    if (container.data) {
        *reinterpret_cast<U *>(container.data) = value;
    }
}
template<class T, class U>
void operator,(T &&value, const ApplyReturnValue<U> &container)
{
    if (container.data) {
        *reinterpret_cast<U *>(container.data) = value;
    }
}

template<class Result>
inline QVariant resultGenerator()
{
    int typeId = qMetaTypeId<Result>();
    return QVariant(QVariant::Type(typeId));
}
template<>
inline QVariant resultGenerator<void>()
{
    return QVariant();
}

template<class Handler>
struct EventHelper;

#define REMOVE_CONST_REF(T) typename std::remove_const<typename std::remove_reference<T>::type>::type

template<class Result, class T>
struct EventHelper<Result (T::*)(void)>
{
    using Func = Result (T::*)(void);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        Q_UNUSED(args)
        QVariant ret = resultGenerator<Result>();
        if (s)
            emit(s->*f)(), ApplyReturnValue<Result>(ret.data());
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1>
struct EventHelper<Result (T::*)(Arg1)>
{
    using Func = Result (T::*)(Arg1);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 1 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2>
struct EventHelper<Result (T::*)(Arg1, Arg2)>
{
    using Func = Result (T::*)(Arg1, Arg2);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 2 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 3 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>(),
                        args.at(2).value<REMOVE_CONST_REF(Arg3)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 4 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>(),
                        args.at(2).value<REMOVE_CONST_REF(Arg3)>(),
                        args.at(3).value<REMOVE_CONST_REF(Arg4)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 5 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>(),
                        args.at(2).value<REMOVE_CONST_REF(Arg3)>(),
                        args.at(3).value<REMOVE_CONST_REF(Arg4)>(),
                        args.at(4).value<REMOVE_CONST_REF(Arg5)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 6 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>(),
                        args.at(2).value<REMOVE_CONST_REF(Arg3)>(),
                        args.at(3).value<REMOVE_CONST_REF(Arg4)>(),
                        args.at(4).value<REMOVE_CONST_REF(Arg5)>(),
                        args.at(5).value<REMOVE_CONST_REF(Arg6)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 7 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>(),
                        args.at(2).value<REMOVE_CONST_REF(Arg3)>(),
                        args.at(3).value<REMOVE_CONST_REF(Arg4)>(),
                        args.at(4).value<REMOVE_CONST_REF(Arg5)>(),
                        args.at(5).value<REMOVE_CONST_REF(Arg6)>(),
                        args.at(6).value<REMOVE_CONST_REF(Arg7)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 8 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>(),
                        args.at(2).value<REMOVE_CONST_REF(Arg3)>(),
                        args.at(3).value<REMOVE_CONST_REF(Arg4)>(),
                        args.at(4).value<REMOVE_CONST_REF(Arg5)>(),
                        args.at(5).value<REMOVE_CONST_REF(Arg6)>(),
                        args.at(6).value<REMOVE_CONST_REF(Arg7)>(),
                        args.at(7).value<REMOVE_CONST_REF(Arg8)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);
    EventHelper(T *self, Func func)
        : s(self), f(func) {}
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 9 && s) {
            emit(s->*f)(args.at(0).value<REMOVE_CONST_REF(Arg1)>(),
                        args.at(1).value<REMOVE_CONST_REF(Arg2)>(),
                        args.at(2).value<REMOVE_CONST_REF(Arg3)>(),
                        args.at(3).value<REMOVE_CONST_REF(Arg4)>(),
                        args.at(4).value<REMOVE_CONST_REF(Arg5)>(),
                        args.at(5).value<REMOVE_CONST_REF(Arg6)>(),
                        args.at(6).value<REMOVE_CONST_REF(Arg7)>(),
                        args.at(7).value<REMOVE_CONST_REF(Arg8)>(),
                        args.at(8).value<REMOVE_CONST_REF(Arg9)>()),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    QPointer<T> s;
    Func f;
};

DPF_END_NAMESPACE

#endif   // EVENTHELPER_H
