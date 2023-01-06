// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLETON_H
#define SINGLETON_H

#include <QObject>
#include <QCoreApplication>
#include <QDebug>

namespace _Singleton {
template<typename T>
static typename QtPrivate::QEnableIf<QtPrivate::AreArgumentsCompatible<T, QObject>::value>::Type
handleQObject(QObject *object)
{
    if (qApp) {
        object->moveToThread(qApp->thread());
    }
}

template<typename T>
static typename QtPrivate::QEnableIf<!QtPrivate::AreArgumentsCompatible<T, QObject>::value>::Type
handleQObject(void*) {}
}

template<typename T>
class Singleton
{
public:
    static T *instance() {
        static T instance;

        if (QtPrivate::AreArgumentsCompatible<T, QObject>::value) {
            _Singleton::handleQObject<T>(&instance);
        }

        return &instance;
    }

private:
    Singleton();
    ~Singleton();
    Singleton(const Singleton &);
    Singleton & operator = (const Singleton &);
};

#endif // SINGLETON_H
