/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef PLUGINSERVICEGLOBAL_H
#define PLUGINSERVICEGLOBAL_H

#include "dfm-framework/dfm_framework_global.h"

#include <QString>
#include <QObject>
#include <QHash>
#include <QtConcurrent>

//全局私有，对内使用

DPF_BEGIN_NAMESPACE

class PluginService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(PluginService)
public:
    explicit PluginService(){}
    virtual ~PluginService(){}
};

namespace GlobalPrivate {

template<class CT>
class QtClassFactory
{
    //定义创建函数类型
    typedef std::function<CT*()> CreateFunc;
public:
    virtual ~QtClassFactory() {}

    template<class T>
    bool regClass(const QString& name, QString * errorString = nullptr)
    {
        if (constructList[name]) {
            if (errorString)
                *errorString = QObject::tr("The current class name has registered "
                                           "the associated construction class");
            return false;
        }

        CreateFunc foo = [=](){
            return dynamic_cast<CT*>(new T());
        };
        constructList.insert(name,foo);
        return true;
    }

    virtual CT* create(const QString& name, QString *errorString = nullptr)
    {
        CreateFunc constantFunc = constructList.value(name);
        if (constantFunc) {
            return constantFunc();
        } else {
            if (errorString)
                *errorString = QObject::tr("Should be call registered 'regClass()' function "
                                           "before create function");
            return nullptr;
        }
    }

protected:
    QHash<QString, CreateFunc> constructList;
};

template<class CT>
class QtClassManager
{
public:

    virtual ~QtClassManager()
    {
        qDeleteAll(classList);
    }

    virtual bool append(const QString& name, CT * obj, QString *errorString = nullptr)
    {
        if (!obj){
            if (errorString)
                *errorString = QObject::tr("Failed, Can't append the empty class pointer");
            return false;
        }

        auto castPointer = qobject_cast<QObject*>(obj);
        if (castPointer)
            castPointer->setParent(nullptr);

        if (classList[name]) {
             if (errorString)
                 *errorString = QObject::tr("Failed, Objects cannot be added repeatedly");
             return false;
        }
        classList.insert(name,obj);
        return true;
    }

    virtual CT * value(const QString &name) const
    {
        auto res = classList[name];
        return res;
    }

    virtual QStringList keys() const
    {
        return classList.keys();
    }

    virtual bool remove(const QString& name)
    {
        auto pointer = classList.take(name);
        if (pointer) delete pointer;
        classList.remove(name);
        return true;
    }

protected:
    QHash<QString, CT*> classList;
};

class PluginServiceGlobal final : public QObject,
        public QtClassFactory<PluginService>,
        public QtClassManager<PluginService>
{
    Q_OBJECT

    inline static PluginServiceGlobal* This = nullptr;

public:

    static PluginServiceGlobal &instance()
    {
        if (!This)
            This  = new PluginServiceGlobal;
        return *This;
    }

private:
    explicit PluginServiceGlobal(){}
};

} //namespace GlobalPrivate

DPF_END_NAMESPACE

#endif // PLUGINSERVICEGLOBAL_H
