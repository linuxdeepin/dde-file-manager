/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
#ifndef SCHEMEFACTORY_H
#define SCHEMEFACTORY_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/abstractfileinfo.h"
#include "dfm-base/base/abstractfilewatcher.h"
#include "dfm-base/base/abstractdiriterator.h"
#include "dfm-base/base/abstractfiledevice.h"
#include "dfm-base/base/private/infocache.h"
#include "dfm-base/base/private/watchercache.h"

#include <dfmio_register.h>

#include <QSharedPointer>
#include <QDirIterator>

#include <functional>

DFMBASE_BEGIN_NAMESPACE

template <class CT> class GC;
/**
 * @brief The class SchemeFactory
 *  根据Scheme注册Class的工厂类，
 *  可使用Scheme进行类构造，前提是当前类需要满足参数
 *  const QUrl &url的构造函数，否则该类将不适用于场景
 * @tparam T 顶层基类
 */
template <class T>
class SchemeFactory
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
        if(!UrlRoute::hasScheme(url.scheme())) {
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


class InfoFactory final : public SchemeFactory<AbstractFileInfo>
{
    Q_DISABLE_COPY(InfoFactory)
    friend class GC<InfoFactory>;
    inline static InfoFactory *ins = nullptr;
public:

    template<class CT = AbstractFileInfo>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractFileInfo>
                ::regClass<CT>(scheme, errorString);
    }

    //提供任意子类的转换方法模板，仅限DAbstractFileInfo树族，
    //与qSharedPointerDynamicCast保持一致
    template<class T>
    static QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        QSharedPointer<AbstractFileInfo> info = InfoCache::instance().getCacheInfo(url);
        if (!info) {
            info = instance().SchemeFactory<AbstractFileInfo>::create(url, errorString);
            InfoCache::instance().cacheInfo(url, info);
        }
        return qSharedPointerDynamicCast<T>(info);
    }

private:
    static InfoFactory& instance(); //获取全局实例
    explicit InfoFactory(){}
};

class WacherFactory final : public SchemeFactory<AbstractFileWatcher>
{
    Q_DISABLE_COPY(WacherFactory)
    friend class GC<WacherFactory>;
    inline static WacherFactory *ins = nullptr;
public:

    template<class CT = AbstractFileWatcher>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractFileWatcher>
                ::regClass<CT>(scheme, errorString);
    }

    //提供任意子类的转换方法模板，仅限DAbstractFileWatcher树族，
    //与qSharedPointerDynamicCast保持一致
    template<class T>
    static QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(url);
        if (!watcher) {
            watcher = instance().SchemeFactory<AbstractFileWatcher>::create(url, errorString);
            WatcherCache::instance().cacheWatcher(url, watcher);
        }
        return qSharedPointerDynamicCast<T>(watcher);
    }

private:
    static WacherFactory& instance();//获取全局实例
    explicit WacherFactory(){}
};

//参数列表偏特化
template<class T = AbstractDirIterator>
class DirIteratorFactoryT1: public SchemeFactory<T>
{
    Q_DISABLE_COPY(DirIteratorFactoryT1)

    //定义多参数构造函数类型
    typedef std::function<QSharedPointer<T>(const QUrl &url, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags)> CreateFuncAgu;

    //构造函数列表
    QHash<QString, CreateFuncAgu> _constructAguList{};

public:

    DirIteratorFactoryT1(){}

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
        if(!UrlRoute::hasScheme(url.scheme())) {
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
        return qSharedPointerDynamicCast<RT>(SchemeFactory<AbstractDirIterator>::create(url, errorString));
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
        return qSharedPointerDynamicCast<RT>(DirIteratorFactoryT1<T>::create(
                                                 url, nameFilters, filters, flags, errorString));
    }
};

class DirIteratorFactory final : public DirIteratorFactoryT1<AbstractDirIterator>
{
    Q_DISABLE_COPY(DirIteratorFactory)
    friend class GC<DirIteratorFactory>;
    inline static DirIteratorFactory *ins = nullptr;
public:

    /* @method regClass
     * @brief 注册Class与Scheme的关联
     * @tparam CT = T 默认传递构造为顶层基类
     * @param const QString &scheme 传递scheme进行类构造绑定
     * @param QString *errorString 错误信息赋值的字符串
     * @return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    template<class CT = AbstractDirIterator>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().DirIteratorFactoryT1<AbstractDirIterator>
                ::create<CT>(scheme,errorString);
    }

    //提供任意子类的转换方法模板，仅限DAbstractFileDevice树族
    //与qSharedPointerDynamicCast保持一致
    template<class RT>
    static QSharedPointer<RT> create(const QUrl &url, QString *errorString = nullptr)
    {
        return instance().DirIteratorFactoryT1<AbstractDirIterator>
                ::create<RT>(url, errorString);
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
    template<class RT = AbstractDirIterator>
    static QSharedPointer<RT> create(const QUrl &url,
                                     const QStringList &nameFilters = QStringList(),
                                     QDir::Filters filters = QDir::NoFilter,
                                     QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                     QString *errorString = nullptr)
    {
        return instance().DirIteratorFactoryT1<AbstractDirIterator>
                ::create<RT>(url, nameFilters, filters, flags, errorString);
    }

private:
    DirIteratorFactory(){}
    static DirIteratorFactory &instance(); //获取全局实例
};

class FileDeviceFactory final : public SchemeFactory<AbstractFileDevice>
{
    Q_DISABLE_COPY(FileDeviceFactory)
    friend class GC<FileDeviceFactory>;
    inline static FileDeviceFactory *ins = nullptr;
public:
    //提供任意子类的转换方法模板，仅限DAbstractFileDevice树族
    //与qSharedPointerDynamicCast保持一致
    template<class CT>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractFileDevice>::regClass<CT>(scheme, errorString);
    }

    template<class T>
    static QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        return qSharedPointerDynamicCast<T>(
                    instance().SchemeFactory<AbstractFileDevice>::create(url, errorString));
    }

private:
    FileDeviceFactory(){}
    static FileDeviceFactory &instance(); //获取全局实例
};

DFMBASE_END_NAMESPACE

#endif // SCHEMEFACTORY_H
