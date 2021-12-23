/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/interfaces/abstractbaseview.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/interfaces/abstractdiriterator.h"
#include "dfm-base/interfaces/private/infocache.h"
#include "dfm-base/interfaces/private/watchercache.h"
#include "dfm-base/utils/finallyutil.h"

#include <dfmio_register.h>

#include <QSharedPointer>
#include <QDirIterator>
#include <QListView>
#include <QDebug>

#include <functional>

DFMBASE_BEGIN_NAMESPACE

template<class CT>
class GC;

/*!
 * \class SchemeFactory
 * \brief 根据Scheme注册Class的工厂类，
 *  可使用Scheme进行类构造，前提是当前类需要满足参数
 *  const QUrl &url的构造函数，否则该类将不适用于场景
 * \tparam T 顶层基类
 */
template<class T>
class SchemeFactory
{

public:
    // 定义创建函数类型
    typedef std::function<QSharedPointer<T>(const QUrl &url)> CreateFunc;

protected:
    // 构造函数列表
    QHash<QString, CreateFunc> constructList {};

public:
    /*!
     * \method regClass
     * \brief 注册Class与Scheme的关联
     * \param CT = T 默认传递构造为顶层基类
     * \param scheme 传递scheme进行类构造绑定
     * \param errorString 错误信息赋值的字符串
     * \return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    template<class CT = T>
    bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        QString error;
        FinallyUtil finally([&]() { if (errorString) *errorString = error; });

        if (constructList[scheme]) {
            error = QObject::tr("The current scheme has registered "
                                "the associated construction class");
            return false;
        }

        CreateFunc foo = [=](const QUrl &url) {
            return QSharedPointer<T>(new CT(url));
        };
        constructList.insert(scheme, foo);
        finally.dismiss();
        return true;
    }

    /*!
     * \method create
     * \brief 根据不同的Url进行顶层类构造，调用该函数存在前置条件
     * 否则将创建空指针
     * 首先需要注册scheme到DFMUrlRoute类
     * 其次需要注册scheme到DFMSchemeFactory<T>类
     * \param url 需要构造的Url
     * \param errorString 错误信息赋值的字符串
     * \return QSharedPointer<T> 动态指针类型，顶层类的抽象接口
     * 如果没有注册 scheme 到 DFMUrlRoute，返回空指针
     * 如果没有注册 scheme 与 class 构造函数规则，返回空指针
     */
    QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        QString error;
        FinallyUtil finally([&]() { if (errorString) *errorString = error; });

        if (!UrlRoute::hasScheme(url.scheme())) {
            error = QObject::tr("No scheme found for "
                                "URL registration");
            return nullptr;
        }

        QString &&scheme = url.scheme();
        CreateFunc constantFunc = constructList.value(scheme);
        if (!constantFunc) {
            error = QObject::tr("Scheme should be call registered 'regClass()' function "
                                "before create function");
            return nullptr;
        }
        finally.dismiss();
        return QSharedPointer<T>(constantFunc(url));
    }
};

class InfoFactory final : public SchemeFactory<AbstractFileInfo>
{
    Q_DISABLE_COPY(InfoFactory)
    friend class GC<InfoFactory>;
    static InfoFactory *ins;

public:
    template<class CT = AbstractFileInfo>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractFileInfo>::regClass<CT>(scheme, errorString);
    }

    // 提供任意子类的转换方法模板，仅限DAbstractFileInfo树族，
    // 与qSharedPointerDynamicCast保持一致
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
    static InfoFactory &instance();   // 获取全局实例
    explicit InfoFactory() {}
};

class ViewFactory final : public SchemeFactory<AbstractBaseView>
{
    Q_DISABLE_COPY(ViewFactory)
    friend class GC<ViewFactory>;
    static ViewFactory *ins;

public:
    template<class CT = AbstractBaseView>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractBaseView>::regClass<CT>(scheme, errorString);
    }

    template<class T>
    static QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractBaseView>::create(url, errorString);
    }

private:
    static ViewFactory &instance();
    explicit ViewFactory() {}
};

class WacherFactory final : public SchemeFactory<AbstractFileWatcher>
{
    Q_DISABLE_COPY(WacherFactory)
    friend class GC<WacherFactory>;
    static WacherFactory *ins;

public:
    template<class CT = AbstractFileWatcher>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractFileWatcher>::regClass<CT>(scheme, errorString);
    }

    // 提供任意子类的转换方法模板，仅限DAbstractFileWatcher树族，
    // 与qSharedPointerDynamicCast保持一致
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
    static WacherFactory &instance();   // 获取全局实例
    explicit WacherFactory() {}
};

// 参数列表偏特化
template<class T = AbstractDirIterator>
class DirIteratorFactoryT1 : public SchemeFactory<T>
{
    Q_DISABLE_COPY(DirIteratorFactoryT1)

    // 定义多参数构造函数类型
    typedef std::function<QSharedPointer<T>(const QUrl &url, const QStringList &nameFilters,
                                            QDir::Filters filters, QDirIterator::IteratorFlags flags)>
            CreateFuncAgu;

    // 构造函数列表
    QHash<QString, CreateFuncAgu> constructAguList {};

public:
    DirIteratorFactoryT1() {}

    /*!
     * \method regClass
     * \brief 注册Class与Scheme的关联
     * \tparam CT = T 默认传递构造为顶层基类
     * \param const QString &scheme 传递scheme进行类构造绑定
     * \param QString *errorString 错误信息赋值的字符串
     * \return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    template<class CT = T>
    bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        if (constructAguList[scheme]) {
            if (errorString)
                *errorString = QObject::tr("The current scheme has registered "
                                           "the associated construction class");
            qWarning() << errorString;
            return false;
        }

        CreateFuncAgu foo = [=](const QUrl &url,
                                const QStringList &nameFilters = QStringList(),
                                QDir::Filters filters = QDir::NoFilter,
                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags) {
            return QSharedPointer<T>(new CT(url, nameFilters, filters, flags));
        };
        constructAguList.insert(scheme, foo);
        return true;
    }

    /*!
     * \method create
     * \brief 根据不同的Url进行顶层类构造，调用该函数存在前置条件
     *  否则将创建空指针
     *  首先需要注册scheme到DFMUrlRoute类
     *  其次需要注册scheme到DFMSchemeFactory<T>类
     * \param const QUrl &url 需要构造的Url
     * \param QString *errorString 错误信息赋值的字符串
     * \return QSharedPointer<T> 动态指针类型，顶层类的抽象接口
     *  如果没有注册 scheme 到 DFMUrlRoute，返回空指针
     *  如果没有注册 scheme 与 class 构造函数规则，返回空指针
     */
    template<class RT>
    QSharedPointer<RT> create(const QUrl &url,
                              const QStringList &nameFilters = QStringList(),
                              QDir::Filters filters = QDir::NoFilter,
                              QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                              QString *errorString = nullptr)
    {
        if (!UrlRoute::hasScheme(url.scheme())) {
            if (errorString)
                *errorString = QObject::tr("No scheme found for "
                                           "URL registration");
            qWarning() << errorString;
            return nullptr;
        }

        QString scheme = url.scheme();
        CreateFuncAgu constantFunc = constructAguList.value(scheme);
        if (constantFunc) {
            return qSharedPointerDynamicCast<RT>(constantFunc(url, nameFilters, filters, flags));
        } else {
            if (errorString)
                *errorString = QObject::tr("Scheme should be call registered 'regClass()' function "
                                           "before create function");
            qWarning() << errorString;
            return nullptr;
        }
    }

    /**
     * \brief 提供任意子类的转换方法模板，仅限DAbstractFileDevice树族
     * 与qSharedPointerDynamicCast保持一致
     * 从顶层中创建并转换响应的Iterator
     */
    template<class RT>
    QSharedPointer<RT> create(const QUrl &url, QString *errorString = nullptr)
    {
        return create<RT>(url, {}, QDir::NoFilter,
                          QDirIterator::NoIteratorFlags,
                          errorString);
    }
};

class DirIteratorFactory final : public DirIteratorFactoryT1<AbstractDirIterator>
{
    Q_DISABLE_COPY(DirIteratorFactory)
    friend class GC<DirIteratorFactory>;
    static DirIteratorFactory *ins;

public:
    /*!
     * \method regClass
     * \brief 注册Class与Scheme的关联
     * \tparam CT = T 默认传递构造为顶层基类
     * \param const QString &scheme 传递scheme进行类构造绑定
     * \param QString *errorString 错误信息赋值的字符串
     * \return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    template<class CT = AbstractDirIterator>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().DirIteratorFactoryT1<AbstractDirIterator>::regClass<CT>(scheme, errorString);
    }

    // 提供任意子类的转换方法模板，仅限DAbstractFileDevice树族
    // 与qSharedPointerDynamicCast保持一致
    template<class RT>
    static QSharedPointer<RT> create(const QUrl &url, QString *errorString = nullptr)
    {
        return instance().DirIteratorFactoryT1<AbstractDirIterator>::create<RT>(url, errorString);
    }

    /*!
     * \method create
     * \brief 根据不同的Url进行顶层类构造，调用该函数存在前置条件
     *  否则将创建空指针
     *  首先需要注册scheme到DFMUrlRoute类
     *  其次需要注册scheme到DFMSchemeFactory<T>类
     * \param const QUrl &url 需要构造的Url
     * \param QString *errorString 错误信息赋值的字符串
     * \return QSharedPointer<T> 动态指针类型，顶层类的抽象接口
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
        return instance().DirIteratorFactoryT1<AbstractDirIterator>::create<RT>(url, nameFilters, filters, flags, errorString);
    }

private:
    DirIteratorFactory() {}
    static DirIteratorFactory &instance();   // 获取全局实例
};

DFMBASE_END_NAMESPACE

#endif   // SCHEMEFACTORY_H
