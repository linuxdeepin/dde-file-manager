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

#ifndef DFMSCHEMEFACTORY_H
#define DFMSCHEMEFACTORY_H

#include "dfm-base/base/dfmurlroute.h"
#include "dfm-base/base/dabstractfileinfo.h"
#include "dfm-base/base/dabstractfilewatcher.h"
#include "dfm-base/base/dabstractdiriterator.h"
#include "dfm-base/base/dabstractfiledevice.h"

#include "dfm-base/base/private/dfminfocache.h"
#include "dfm-base/base/private/dfmwatchercache.h"

#include <QSharedPointer>
#include <QDirIterator>

#include <functional>

/**
 * @brief The class DFMSchemeFactory
 *  根据Scheme注册Class的工厂类，
 *  可使用Scheme进行类构造，前提是当前类需要满足参数
 *  const QUrl &url的构造函数，否则该类将不适用于场景
 * @tparam T 顶层基类
 */
template <class T>
class DFMSchemeFactory
{

public:
    //定义创建函数类型
    typedef std::function<QSharedPointer<T>(const QUrl &url)> CreateFunc;

protected:
    //构造函数列表
    QHash<QString, CreateFunc> _constructList{};
public:
    /* @method regClass
     * @brief 注册Class与Scheme的关联
     * @tparam CT = T 默认传递构造为顶层基类
     * @param const QString &scheme 传递scheme进行类构造绑定
     * @param QString *errorString 错误信息赋值的字符串
     * @return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    template<class CT = T>
    bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        if (_constructList[scheme]) {
            if (errorString)
                *errorString = QObject::tr("The current scheme has registered "
                                           "the associated construction class");
            return false;
        }

        CreateFunc foo = [=](const QUrl& url){
            return QSharedPointer<T>(new CT(url));
        };
        _constructList.insert(scheme,foo);
        return true;
    }

    /* @method create
     * @brief 根据不同的Url进行顶层类构造，调用该函数存在前置条件
     *  否则将创建空指针
     *  首先需要注册scheme到DFMUrlRoute类
     *  其次需要注册scheme到DFMSchemeFactory<T>类
     * @param const QUrl &url 需要构造的Url
     * @param QString *errorString 错误信息赋值的字符串
     * @return QSharedPointer<T> 动态指针类型，顶层类的抽象接口
     *  如果没有注册 scheme 到 DFMUrlRoute，返回空指针
     *  如果没有注册 scheme 与 class 构造函数规则，返回空指针
     */
    QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        if(!DFMUrlRoute::hasScheme(url.scheme())) {
            if (errorString)
                *errorString = QObject::tr("No scheme found for "
                                           "URL registration");
            return nullptr;
        }

        QString scheme = url.scheme();
        CreateFunc constantFunc = _constructList.value(scheme);
        if (constantFunc) {
            return QSharedPointer<T>(constantFunc(url));
        } else {
            if (errorString)
                *errorString = QObject::tr("Scheme should be call registered 'regClass()' function "
                                           "before create function");
            return nullptr;
        }
    }
};


class DFMInfoFactory final : public DFMSchemeFactory<DAbstractFileInfo>
{
    Q_DISABLE_COPY(DFMInfoFactory)

public:

    DFMInfoFactory(){}

    //提供任意子类的转换方法模板，仅限DAbstractFileInfo树族，
    //与qSharedPointerDynamicCast保持一致
    template<class T>
    QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {

        QSharedPointer<DAbstractFileInfo> info = DFMInfoCache::instance().getCacheInfo(url);
        if (!info) {
            info = DFMSchemeFactory<DAbstractFileInfo>::create(url, errorString);
            DFMInfoCache::instance().cacheInfo(url, info);
        }
        return qSharedPointerDynamicCast<T>(info);
    }

    //获取全局实例
    static DFMInfoFactory& instance();
};

class DFMWacherFactory final : public DFMSchemeFactory<DAbstractFileWatcher>
{
    Q_DISABLE_COPY(DFMWacherFactory)
public:

    DFMWacherFactory(){}

    //提供任意子类的转换方法模板，仅限DAbstractFileWatcher树族，
    //与qSharedPointerDynamicCast保持一致
    template<class T>
    QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        QSharedPointer<DAbstractFileWatcher> watcher = DFMWatcherCache::instance().getCacheWatcher(url);
        if (!watcher) {
            watcher = DFMSchemeFactory<DAbstractFileWatcher>::create(url, errorString);
            DFMWatcherCache::instance().cacheWatcher(url, watcher);
        }
        return qSharedPointerDynamicCast<T>(watcher);
    }

    //获取全局实例
    static DFMWacherFactory& instance();
};

//参数列表偏特化
template<class T = DAbstractDirIterator>
class DFMDirIteratorFactoryT1: public DFMSchemeFactory<T>
{
    Q_DISABLE_COPY(DFMDirIteratorFactoryT1)

    //定义多参数构造函数类型
    typedef std::function<QSharedPointer<T>(const QUrl &url, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags)> CreateFuncAgu;

    //构造函数列表
    QHash<QString, CreateFuncAgu> _constructAguList{};

public:

    DFMDirIteratorFactoryT1(){}

    /* @method regClass
     * @brief 注册Class与Scheme的关联
     * @tparam CT = T 默认传递构造为顶层基类
     * @param const QString &scheme 传递scheme进行类构造绑定
     * @param QString *errorString 错误信息赋值的字符串
     * @return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    template<class CT = T>
    bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        if (_constructAguList[scheme]) {
            if (errorString)
                *errorString = QObject::tr("The current scheme has registered "
                                           "the associated construction class");
            return false;
        }

        CreateFuncAgu foo = [=](const QUrl &url,
                             const QStringList &nameFilters = QStringList(),
                             QDir::Filters filters = QDir::NoFilter,
                             QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags){
            return QSharedPointer<T>(new CT(url, nameFilters, filters, flags));
        };
        _constructAguList.insert(scheme,foo);
        return true;
    }

    /* @method create
     * @brief 根据不同的Url进行顶层类构造，调用该函数存在前置条件
     *  否则将创建空指针
     *  首先需要注册scheme到DFMUrlRoute类
     *  其次需要注册scheme到DFMSchemeFactory<T>类
     * @param const QUrl &url 需要构造的Url
     * @param QString *errorString 错误信息赋值的字符串
     * @return QSharedPointer<T> 动态指针类型，顶层类的抽象接口
     *  如果没有注册 scheme 到 DFMUrlRoute，返回空指针
     *  如果没有注册 scheme 与 class 构造函数规则，返回空指针
     */
    QSharedPointer<T> create(const QUrl &url,
                             const QStringList &nameFilters = QStringList(),
                             QDir::Filters filters = QDir::NoFilter,
                             QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                             QString *errorString = nullptr)
    {
        if(!DFMUrlRoute::hasScheme(url.scheme())) {
            if (errorString)
                *errorString = QObject::tr("No scheme found for "
                                           "URL registration");
            return nullptr;
        }

        QString scheme = url.scheme();
        CreateFuncAgu constantFunc = _constructAguList.value(scheme);
        if (constantFunc) {
            return QSharedPointer<T>(constantFunc(url, nameFilters, filters, flags));
        } else {
            if (errorString)
                *errorString = QObject::tr("Scheme should be call registered 'regClass()' function "
                                           "before create function");
            return nullptr;
        }
    }

    //提供任意子类的转换方法模板，仅限DAbstractFileDevice树族
    //与qSharedPointerDynamicCast保持一致
    template<class RT>
    QSharedPointer<RT> create(const QUrl &url, QString *errorString = nullptr)
    {
        return qSharedPointerDynamicCast<RT>(DFMSchemeFactory<DAbstractDirIterator>::create(url, errorString));
    }

    //提供任意子类的转换方法模板，仅限DAbstractFileDevice树族
    //与qSharedPointerDynamicCast保持一致
    template<class RT>
    QSharedPointer<RT> create(const QUrl &url,
                             const QStringList &nameFilters,
                             QDir::Filters filters = QDir::NoFilter,
                             QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                             QString *errorString = nullptr)
    {
        return qSharedPointerDynamicCast<RT>(DFMDirIteratorFactoryT1<T>::create(
                                                url, nameFilters, filters, flags, errorString));
    }
};

class DFMDirIteratorFactory final : public DFMDirIteratorFactoryT1<DAbstractDirIterator>
{
    Q_DISABLE_COPY(DFMDirIteratorFactory)
public:

    DFMDirIteratorFactory(){}

    //获取全局实例
    static DFMDirIteratorFactory &instance();
};

class DFMFileDeviceFactory final : public DFMSchemeFactory<DAbstractFileDevice>
{
    Q_DISABLE_COPY(DFMFileDeviceFactory)

public:

    DFMFileDeviceFactory(){}

    //提供任意子类的转换方法模板，仅限DAbstractFileDevice树族
    //与qSharedPointerDynamicCast保持一致
    template<class T>
    QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        return qSharedPointerDynamicCast<T>(DFMSchemeFactory<DAbstractFileDevice>::create(url, errorString));
    }

    //获取FileDevice的全局实例
    static DFMFileDeviceFactory& instance();
};

#endif // DFMSCHEMEFACTORY_H
