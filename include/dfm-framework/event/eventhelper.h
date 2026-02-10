// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTHELPER_H
#define EVENTHELPER_H

#include <dfm-framework/dfm_framework_global.h>

#include <QDebug>
#include <QVariant>
#include <QObject>
#include <QUrl>
#include <QThread>
#include <QCoreApplication>

#include <mutex>

DPF_BEGIN_NAMESPACE

using EventType = int;
using EventConverterFunc = std::function<EventType(const QString & /* space */, const QString & /* topic */)>;

enum class EventStratege {
    kSignal,
    kSlot,
    kHook
};
inline constexpr char kSignalStrategePrefix[] { "signal" };
inline constexpr char kSlotStrategePrefix[] { "slot" };
inline constexpr char kHookStrategePrefix[] { "hook" };

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

inline bool isWellKnownEvenType(EventType type)
{
    return type >= EventTypeScope::kWellKnownEventBase && type < EventTypeScope::kWellKnownEventTop;
}

inline void threadEventAlert(const QString &eventName)
{
    if (Q_UNLIKELY(QThread::currentThread() != QCoreApplication::instance()->thread()))
        qCWarning(logDPF) << "[Event Thread]: The event call does not run in the main thread: " << eventName;
}

inline void threadEventAlert(const QString &space, const QString &topic)
{
    threadEventAlert(space + "::" + topic);
}

inline void threadEventAlert(EventType type)
{
    if (!isWellKnownEvenType(type))
        return;
    threadEventAlert(QString::number(type));
}

class EventConverter
{
public:
    static inline EventConverterFunc convertFunc {};
    static void registerConverter(const EventConverterFunc &func)
    {
        static std::once_flag flag;
        std::call_once(flag, [&func]() {
            convertFunc = func;
        });
    }
    static EventType convert(const QString &space, const QString &topic)
    {
        if (convertFunc)
            return convertFunc(space, topic);
        return EventTypeScope::kInValid;
    }
};

/*
 * Check return value type
 */
template<class T>
struct ReturnTypeHelper;

template<class R, class... Args>
struct ReturnTypeHelper<R(Args...)>
{
    using type = R;
};

template<class R, class... Args>
struct ReturnTypeHelper<R (*)(Args...)>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...)>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) &>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) &&>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const &>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const &&>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) volatile>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) volatile &>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) volatile &&>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const volatile>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const volatile &>
{
    using type = R;
};

template<class R, class C, class... Args>
struct ReturnTypeHelper<R (C::*)(Args...) const volatile &&>
{
    using type = R;
};

template<class T>
using ReturnType = typename ReturnTypeHelper<T>::type;

/*
 * copy from qobjectdefs_impl.h and kylinpluginframewrok
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
        : data(data_) { }
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

#define REMOVE_CONST_REF(T) typename std::remove_const<typename std::remove_reference<T>::type>::type

template<class Result>
inline QVariant resultGenerator()
{
    int typeId = qMetaTypeId<Result>();
    return QVariant(QMetaType(typeId));
}
template<>
inline QVariant resultGenerator<void>()
{
    return QVariant();
}

template<class T>
inline REMOVE_CONST_REF(T) paramGenerator(const QVariant &param)
{
// NOTE: Defining a std::function type using Q_DECLARE_METATYPE
// in a different header file for the MIPS platform will result
// in a different `MetaTypeId`, which will cause `qvariant_cast<T>` to fail
#ifdef ARCH_MIPSEL
    if constexpr (std::is_class_v<REMOVE_CONST_REF(T)> && !std::is_base_of<QObject, REMOVE_CONST_REF(T)>::value
                  && !std::is_null_pointer_v<REMOVE_CONST_REF(T)>) {
        auto rawPtr { reinterpret_cast<const REMOVE_CONST_REF(T) *>(param.constData()) };
        const QString &name { typeid(T).name() };
        if (name.contains("function") && !param.isNull() && param.isValid() && rawPtr)
            return *rawPtr;
    }
#endif
    return param.value<REMOVE_CONST_REF(T)>();
}

template<class Handler>
struct EventHelper;

template<class Result, class T>
struct EventHelper<Result (T::*)(void)>
{
    using Func = Result (T::*)(void);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        Q_UNUSED(args)
        QVariant ret = resultGenerator<Result>();
        if (s)
            emit(s->*f)(), ApplyReturnValue<Result>(ret.data());
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1>
struct EventHelper<Result (T::*)(Arg1)>
{
    using Func = Result (T::*)(Arg1);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 1) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2>
struct EventHelper<Result (T::*)(Arg1, Arg2)>
{
    using Func = Result (T::*)(Arg1, Arg2);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 2) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 3) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1)),
                        paramGenerator<Arg3>(args.at(2))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 4) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1)),
                        paramGenerator<Arg3>(args.at(2)),
                        paramGenerator<Arg4>(args.at(3))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 5) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1)),
                        paramGenerator<Arg3>(args.at(2)),
                        paramGenerator<Arg4>(args.at(3)),
                        paramGenerator<Arg5>(args.at(4))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 6) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1)),
                        paramGenerator<Arg3>(args.at(2)),
                        paramGenerator<Arg4>(args.at(3)),
                        paramGenerator<Arg5>(args.at(4)),
                        paramGenerator<Arg6>(args.at(5))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 7) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1)),
                        paramGenerator<Arg3>(args.at(2)),
                        paramGenerator<Arg4>(args.at(3)),
                        paramGenerator<Arg5>(args.at(4)),
                        paramGenerator<Arg6>(args.at(5)),
                        paramGenerator<Arg7>(args.at(6))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 8) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1)),
                        paramGenerator<Arg3>(args.at(2)),
                        paramGenerator<Arg4>(args.at(3)),
                        paramGenerator<Arg5>(args.at(4)),
                        paramGenerator<Arg6>(args.at(5)),
                        paramGenerator<Arg7>(args.at(6)),
                        paramGenerator<Arg8>(args.at(7))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

template<class Result, class T, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9>
struct EventHelper<Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9)>
{
    using Func = Result (T::*)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);
    EventHelper(T *self, Func func)
        : s(self), f(func) { }
    QVariant invoke(const QVariantList &args)
    {
        QVariant ret = resultGenerator<Result>();
        if (args.count() == 9) {
            emit(s->*f)(paramGenerator<Arg1>(args.at(0)),
                        paramGenerator<Arg2>(args.at(1)),
                        paramGenerator<Arg3>(args.at(2)),
                        paramGenerator<Arg4>(args.at(3)),
                        paramGenerator<Arg5>(args.at(4)),
                        paramGenerator<Arg6>(args.at(5)),
                        paramGenerator<Arg7>(args.at(6)),
                        paramGenerator<Arg8>(args.at(7)),
                        paramGenerator<Arg9>(args.at(8))),
                    ApplyReturnValue<Result>(ret.data());
        }
        return ret;
    }

protected:
    T *s;
    Func f;
};

/*
 * cast member function to void *
 */
template<class T, class R>
void *memberFunctionVoidCast(R (T::*f)(void))
{
    union {
        R(T::*pf)
        ();
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1>
void *memberFunctionVoidCast(R (T::*f)(Arg1))
{
    union {
        R(T::*pf)
        (Arg1);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2))
{
    union {
        R(T::*pf)
        (Arg1, Arg2);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2, class Arg3>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2, Arg3))
{
    union {
        R(T::*pf)
        (Arg1, Arg2, Arg3);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2, class Arg3, class Arg4>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2, Arg3, Arg4))
{
    union {
        R(T::*pf)
        (Arg1, Arg2, Arg3, Arg4);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2, Arg3, Arg4, Arg5))
{
    union {
        R(T::*pf)
        (Arg1, Arg2, Arg3, Arg4, Arg5);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
{
    union {
        R(T::*pf)
        (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7))
{
    union {
        R(T::*pf)
        (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8))
{
    union {
        R(T::*pf)
        (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);
        void *p;
    };
    pf = f;
    return p;
}

template<class T, class R, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9>
void *memberFunctionVoidCast(R (T::*f)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9))
{
    union {
        R(T::*pf)
        (Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);
        void *p;
    };
    pf = f;
    return p;
}

/*
 * index to the event handler
 */
template<class Method>
struct EventHandler
{
    QObject *objectIndex;
    // Warning: pointers to member functions and pointers to data are not necessarily represented
    // in the same way. A pointer to a member function might be a data structure rather than a single pointer.
    // Think about it: if it’s pointing at a virtual function, it might not actually be pointing at a statically resolvable pile of code,
    // so it might not even be a normal address — it might be a different data structure of some sort.
    // See: https://stackoverflow.com/questions/1307278/casting-between-void-and-a-pointer-to-member-function
    void *funcIndex;
    Method handler;

    inline EventHandler(QObject *obj, void *func, Method method)
        : objectIndex(obj),
          funcIndex(func),
          handler(method)
    {
    }

    inline bool compare(QObject *obj)
    {
        if (!objectIndex)
            return false;
        return obj == objectIndex;
    }

    template<class Func>
    inline bool compare(QObject *obj, Func func)
    {
        if (!objectIndex || !funcIndex)
            return false;
        return compare(obj) && memberFunctionVoidCast(func) == funcIndex;
    }

    inline bool operator==(const EventHandler &rhs) const
    {
        return objectIndex == rhs.objectIndex
                && funcIndex == rhs.funcIndex;
    }
};

DPF_END_NAMESPACE

#endif   // EVENTHELPER_H
