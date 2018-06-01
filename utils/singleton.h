/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
