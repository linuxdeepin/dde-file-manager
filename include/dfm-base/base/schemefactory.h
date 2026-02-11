// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCHEMEFACTORY_H
#define SCHEMEFACTORY_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/infocache.h>
#include <dfm-base/utils/watchercache.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractbaseview.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/interfaces/abstractsortfilter.h>

#include <QCoreApplication>
#include <QSharedPointer>
#include <QDirIterator>
#include <QListView>
#include <QDebug>

#include <functional>

namespace dfmbase {

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
    typedef std::function<T*(const QUrl &url)> CreatePointerFunc;
    using TransFunc = std::function<QSharedPointer<T>(QSharedPointer<T>)>;

protected:
    // 构造函数列表
    DThreadMap<QString, CreateFunc> constructList {};
    // 转换函数列表
    DThreadMap<QString, TransFunc> transList {};
    // 构造函数列表
    DThreadMap<QString, CreatePointerFunc> constructPointerList {};

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
        CreateFunc foo = [=](const QUrl &url) {
            return QSharedPointer<T>(new CT(url));
        };
        return regCreator(scheme, foo, errorString);
    }

    bool transClass(const QString &scheme, TransFunc func, QString *errorString = nullptr)
    {
        QString error;
        FinallyUtil finally([&]() { if (errorString) *errorString = error; });

        if (transList.contains(scheme)) {
            error = "The current scheme trans func has registered";
            return false;
        }

        transList.insert(scheme, func);
        finally.dismiss();
        return true;
    }

    /*!
     * \method regCreator
     * \brief 注册Class创建器与Scheme的关联
     * \param scheme 传递scheme进行类构造绑定
     * \param creator 创建函数
     * \param errorString 错误信息赋值的字符串
     * \return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    bool regCreator(const QString &scheme, CreateFunc creator, QString *errorString = nullptr)
    {
        QString error;
        FinallyUtil finally([&]() { if (errorString) *errorString = error; });

        if (constructList.contains(scheme)) {
            error = "The current scheme has registered "
                    "the associated construction class";
            return false;
        }

        constructList.insert(scheme, creator);
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
        return create(url.scheme(), url, errorString);
    }

    QSharedPointer<T> create(const QString &scheme, const QUrl &url, QString *errorString = nullptr)
    {
        QString error;
        FinallyUtil finally([&]() { if (errorString) *errorString = error; });

        if (!UrlRoute::hasScheme(scheme)) {
            error = "No scheme found for "
                    "URL registration";
            return nullptr;
        }

        CreateFunc constantFunc = constructList.value(scheme);
        if (!constantFunc) {
            error = "Scheme should be call registered 'regClass()' function "
                    "before create function";
            return nullptr;
        }
        finally.dismiss();
        QSharedPointer<T> info = QSharedPointer<T>(constantFunc(url));

        TransFunc func = transList.value(url.scheme());
        if (func)
            info = func(info);

        return info;
    }

    /*!
     * \method regClassPointer
     * \brief 注册Class与Scheme的关联
     * \param CT = T 默认传递构造为顶层基类
     * \param scheme 传递scheme进行类构造绑定
     * \param errorString 错误信息赋值的字符串
     * \return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    template<class CT = T>
    bool regClassPointer(const QString &scheme, QString *errorString = nullptr)
    {
        CreatePointerFunc foo = [=](const QUrl &url) {
            return new CT(url);
        };
        return regPointerCreator(scheme, foo, errorString);
    }

    /*!
     * \method regPointerCreator
     * \brief 注册Class创建器与Scheme的关联
     * \param scheme 传递scheme进行类构造绑定
     * \param creator 创建函数
     * \param errorString 错误信息赋值的字符串
     * \return bool 注册结果，如果当前已存在scheme的关联，则返回false
     *  否则返回true
     */
    bool regPointerCreator(const QString &scheme, CreatePointerFunc creator, QString *errorString = nullptr)
    {
        QString error;
        FinallyUtil finally([&]() { if (errorString) *errorString = error; });

        if (constructPointerList.contains(scheme)) {
            error = "The current scheme has registered "
                    "the associated construction class";
            return false;
        }

        constructPointerList.insert(scheme, creator);
        finally.dismiss();
        return true;
    }
    /*!
     * \method createPointer
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
    T* createPointer(const QUrl &url, QString *errorString = nullptr)
    {
        return createPointer(url.scheme(), url, errorString);
    }

    T *createPointer(const QString &scheme, const QUrl &url, QString *errorString = nullptr)
    {
        QString error;
        FinallyUtil finally([&]() { if (errorString) *errorString = error; });

        if (!UrlRoute::hasScheme(scheme)) {
            error = "No scheme found for "
                    "URL registration";
            return nullptr;
        }

        CreatePointerFunc constantFunc = constructPointerList.value(scheme);
        if (!constantFunc) {
            error = "Scheme should be call registered 'regClass()' function "
                    "before create function";
            return nullptr;
        }
        finally.dismiss();
        return constantFunc(url);
    }

    // cache fileinfo in enumerator, transform fileinfo to desktop fileinfo in need
    QSharedPointer<T> transformInfo(const QString &scheme, QSharedPointer<T> info)
    {
        TransFunc func = transList.value(scheme);
        if (func)
            info = func(info);

        return info;
    }
};

class InfoFactory final : public SchemeFactory<FileInfo>
{
    Q_DISABLE_COPY(InfoFactory)

public:
    enum RegOpts : uint32_t {
        kNoOpt = 0,
        kNoCache = 1
        // add other opt like `kAnyOpt = 1 << 1 ... n`
    };

    template<class CT = FileInfo>
    static bool regClass(const QString &scheme, RegOpts opts = RegOpts::kNoOpt, QString *errorString = nullptr)
    {
        assert(qApp->thread() == QThread::currentThread());
        static std::once_flag createFlag;
        // main thread create InfoCacheController
        std::call_once(createFlag, [] {
            InfoCacheController::instance();
        });
        if (opts & RegOpts::kNoCache)
            InfoCacheController::instance().setCacheDisbale(scheme);
        return instance().SchemeFactory<FileInfo>::regClass<CT>(scheme, errorString);
    }

    template<class CT>
    static bool regInfoTransFunc(const QString &scheme, std::function<QSharedPointer<CT>(QSharedPointer<CT>)> func)
    {
        return instance().SchemeFactory<FileInfo>::transClass(scheme, func);
    }

    static bool regCreator(const QString &scheme, CreateFunc creator, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<FileInfo>::regCreator(scheme, creator, errorString);
    }

    template<class T>
    static QSharedPointer<T> transfromInfo(const QString &scheme, QSharedPointer<T> info)
    {
        return instance().SchemeFactory<FileInfo>::transformInfo(scheme, info);
    }

    // 提供任意子类的转换方法模板，仅限DAbstractFileInfo树族，
    // 与qSharedPointerDynamicCast保持一致
    template<class T>
    static QSharedPointer<T> create(const QUrl &url,
                                    const Global::CreateFileInfoType type = Global::CreateFileInfoType::kCreateFileInfoAuto,
                                    QString *errorString = nullptr)
    {
        if (!url.isValid()) {
            qCWarning(logDFMBase) << "url is invalid !!! url = " << url;
            return nullptr;
        }

        if (InfoCacheController::instance().cacheDisable(url.scheme()))
            return qSharedPointerDynamicCast<T>(instance().SchemeFactory<FileInfo>::
                                                        create(url, errorString));

        if (type == Global::CreateFileInfoType::kCreateFileInfoSyncAndCache)
            return qSharedPointerDynamicCast<T>(getFileInfoFromCache(url, Global::CreateFileInfoType::kCreateFileInfoSyncAndCache, errorString));

        if (type == Global::CreateFileInfoType::kCreateFileInfoAsyncAndCache && url.scheme() == Global::Scheme::kFile)
            return qSharedPointerDynamicCast<T>(getFileInfoFromCache(url, Global::CreateFileInfoType::kCreateFileInfoAsyncAndCache, errorString));

        if (url.scheme() == Global::Scheme::kFile) {
            if (type == Global::CreateFileInfoType::kCreateFileInfoSync) {
                return qSharedPointerDynamicCast<T>(instance().SchemeFactory<FileInfo>::
                                                            create(url, errorString));
            } else if (type == Global::CreateFileInfoType::kCreateFileInfoAsync) {
                auto info = qSharedPointerDynamicCast<T>(instance().SchemeFactory<FileInfo>::
                                                                 create(Global::Scheme::kAsyncFile, url, errorString));
                if (info)
                    info->updateAttributes();
                return info;
            }
        }

        QSharedPointer<FileInfo> info = InfoCacheController::instance().getCacheInfo(url);
        if (!info) {
            auto tarScheme = scheme(url);
            info = instance().SchemeFactory<FileInfo>::create(tarScheme, url, errorString);
            if (info && tarScheme == Global::Scheme::kAsyncFile)
                info->updateAttributes();

            if (type != Global::CreateFileInfoType::kCreateFileInfoAutoNoCache)
                emit InfoCacheController::instance().cacheFileInfo(url, info);
        }

        if (!info)
            qCWarning(logDFMBase) << "info is nullptr url = " << url;

        return qSharedPointerDynamicCast<T>(info);
    }

    static void cacheFileInfo(const QSharedPointer<FileInfo> &info) {
        if (InfoCacheController::instance().cacheDisable(info->fileUrl().scheme()))
            return;
        emit InfoCacheController::instance().removeCacheFileInfo({info->fileUrl()});
        emit InfoCacheController::instance().cacheFileInfo(info->fileUrl(), info);
    }

private:
    static InfoFactory &instance();   // 获取全局实例
    explicit InfoFactory() {}
    static QString scheme(const QUrl &url);
    static QSharedPointer<FileInfo> getFileInfoFromCache(const QUrl &url, Global::CreateFileInfoType type, QString *errorString);
};

class ViewFactory final : public SchemeFactory<AbstractBaseView>
{
    Q_DISABLE_COPY(ViewFactory)

public:
    template<class CT = AbstractBaseView>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractBaseView>::regClassPointer<CT>(scheme, errorString);
    }

    template<class T>
    static T* create(const QUrl &url, QString *errorString = nullptr)
    {
        auto view = instance().SchemeFactory<AbstractBaseView>::createPointer(url, errorString);
        return view;
    }

private:
    static ViewFactory &instance();
    explicit ViewFactory() {}
};

class WatcherFactory final : public SchemeFactory<AbstractFileWatcher>
{
    Q_DISABLE_COPY(WatcherFactory)

public:
    enum RegOpts : uint32_t {
        kNoOpt = 0,
        kNoCache = 1
        // add other opt like `kAnyOpt = 1 << 1 ... n`
    };

    template<class CT = AbstractFileWatcher>
    static bool regClass(const QString &scheme, RegOpts opts = RegOpts::kNoOpt, QString *errorString = nullptr)
    {
        if (opts & RegOpts::kNoCache)
            WatcherCache::instance().setCacheDisbale(scheme);
        return instance().SchemeFactory<AbstractFileWatcher>::regClass<CT>(scheme, errorString);
    }

    // 提供任意子类的转换方法模板，仅限DAbstractFileWatcher树族，
    // 与qSharedPointerDynamicCast保持一致
    template<class T>
    static QSharedPointer<T> create(const QUrl &url, const bool cache = true, QString *errorString = nullptr)
    {
        if (Q_UNLIKELY(!cache) || WatcherCache::instance().cacheDisable(url.scheme()))
            return qSharedPointerDynamicCast<T>(instance().SchemeFactory<AbstractFileWatcher>::create(url, errorString));

        QSharedPointer<AbstractFileWatcher> watcher = WatcherCache::instance().getCacheWatcher(url);
        if (watcher.isNull()) {
            watcher = instance().SchemeFactory<AbstractFileWatcher>::create(url, errorString);
            if (watcher) {
                watcher->moveToThread(qApp->thread());
                WatcherCache::instance().cacheWatcher(url, watcher);
            }
        }
        return qSharedPointerDynamicCast<T>(watcher);
    }

private:
    static WatcherFactory &instance();   // 获取全局实例
    explicit WatcherFactory() {}
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
                *errorString = "The current scheme has registered "
                               "the associated construction class";
            qCWarning(logDFMBase) << errorString;
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
                *errorString = "No scheme found for "
                               "URL registration";
            qCWarning(logDFMBase) << errorString;
            return nullptr;
        }

        QString scheme = url.scheme();
        CreateFuncAgu constantFunc = constructAguList.value(scheme);
        if (constantFunc) {
            return qSharedPointerDynamicCast<RT>(constantFunc(url, nameFilters, filters, flags));
        } else {
            if (errorString)
                *errorString = "Scheme should be call registered 'regClass()' function "
                               "before create function";
            qCWarning(logDFMBase) << errorString;
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

class SortFilterFactory final : public SchemeFactory<AbstractSortFilter>
{
    Q_DISABLE_COPY(SortFilterFactory)

public:
    template<class CT = AbstractSortFilter>
    static bool regClass(const QString &scheme, QString *errorString = nullptr)
    {
        return instance().SchemeFactory<AbstractSortFilter>::regClass<CT>(scheme, errorString);
    }

    template<class T>
    static QSharedPointer<T> create(const QUrl &url, QString *errorString = nullptr)
    {
        auto sortFilters = instance().SchemeFactory<AbstractSortFilter>::create(url, errorString);
        return qSharedPointerDynamicCast<T>(sortFilters);
    }

private:
    static SortFilterFactory &instance();   // 获取全局实例
    explicit SortFilterFactory() {}
};
}   // namespace dfmbase

#endif   // SCHEMEFACTORY_H
