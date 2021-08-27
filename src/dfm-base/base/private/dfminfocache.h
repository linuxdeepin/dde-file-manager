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

#ifndef DFMINFOCACHESMANAGER_H
#define DFMINFOCACHESMANAGER_H

#include "dfm-base/utils/threadcontainer.hpp"

#include "dfm-base/base/dabstractfileinfo.h"

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QUrl>

/// 这个类是对Url的强对应，必须保证url的唯一性
/// 一、缓存规则
///     1.fileinfo创建时缓存它的共享指针
///     2.刷新它的refreshtime
/// 二、刷新规则
///     1.收到刷新信号，判断文件的refreshtime是否大于给定值500ms
///     2.大于直接刷新
///     3.小于不刷新，插入到待刷新队列
///     4.开启刷新线程，每500ms遍历一次待刷新队列
/// 三、析构规则
///     1.启动一个5分钟定时器
///     2.每5分钟遍历缓存查询，将共享指针引用计数小于1的缓存文件信息的url加入到待析构队列
///     3.当每次getCacheInfo时，移除在待析构队列的url
///     4.启动一个333ms延迟于前一个计时器的定时器
///     5.每5分钟遍历待析构队里，移除cacheinfo
///     6.补充，当文件缓存数量操过100000，移除前面的缓存

// 异步刷新线程
class ReFreshThread : public QThread
{
    Q_OBJECT
public:
    explicit ReFreshThread(QObject *parent = nullptr);
    /**
     * @brief refreshFileInfoByUrl 根据URL刷新fileinfo
     * 判断fileinfo中上次更新的时间是否超过更新间隔时间，超过立即更新，不然加入待更新链表
     * @param QUrl 文件的URL
     * @return
     */
    void refreshFileInfoByUrl(const QUrl &url);
    /**
     * @brief updateRefreshTimeByUrl 根据URL更新文件的刷新事件
     * 判断上次更新的时间是否超过更新间隔时,跟新刷新时间，不然加入待更map
     * @param QUrl 文件的URL
     * @return
     */
    void updateRefreshTimeByUrl(const QUrl &url);
    /**
     * @brief stopRefresh 停止更新线程
     * 设置为停止状态，并且阻塞等待线程结束
     * @return
     */
    void stopRefresh();
    /**
     * @brief removeRefreshByUrl 根据URL移除更新
     * 移除更新时间信息和待更新信息，一般是在移除fileinfo，cache析构的时候
     * @param QUrl 文件的URL
     * @return
     */
    void removeRefreshByUrl(const QUrl &url);

protected:
    /**
     * @brief run 线程执行函数
     * 固定时间去遍历待更新list，判断fileinfo中上次更新的时间是否超过更新间隔时间，
     * 超过立即更新，否则退出当前遍历，线程沉睡更新间隔时间
     * @return
     */
    void run() override;

private:
    //待更新链表
    DThreadMap<QUrl, qint64> m_needRefreshMap;
    //刷新时间
    DThreadMap<QUrl, qint64> m_refreshMap;
    //已被removecache的url
    DThreadList<QUrl> m_removedCacheList;
    //计时器
    QTime time;
    //停止标志
    QAtomicInteger<bool> m_bStop = false;
};


class DFMInfoFactory;
class DFMInfoCachePrivate;
class DFMInfoCache : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFMInfoCache)
    QScopedPointer<DFMInfoCachePrivate> d_ptr;
    friend DFMInfoFactory;
    friend ReFreshThread;

public:
    explicit DFMInfoCache(QObject *parent = nullptr);
    virtual ~DFMInfoCache();

private:
    /**
     * @brief instance 获取DFileInfoManager的对象
     * @return DFileInfoManager的对象
     */
    static DFMInfoCache &instance();
    /**
     * @brief getFileInfo 获取fileinfo
     * 获取缓存的fileinfo的智能指针，不需要自己析构和refresh
     * @param QUrl 文件的URL
     * @return fileinfo的智能指针
     */
    DAbstractFileInfoPointer getCacheInfo(const QUrl &url);
    /**
     * @brief cacheInfo 缓存fileinfo
     * 线程安全，缓存fileinfo，又就返回，没有就缓存
     * @param DAbstractFileInfoPointer fileinfo的智能指针
     * @return
     */
    void cacheInfo(const QUrl &url, const DAbstractFileInfoPointer &info);
    /**
     * @brief removeCacheInfo 移除缓存的fileinfo
     * 线程安全，缓存fileinfo，有就移除，没有就返回
     * @param QUrl 文件的URL
     * @param DAbstractFileInfoPointer fileinfo的智能指针
     * @return
     */
    void removeCacheInfo(const QUrl &url);

private Q_SLOTS:
    /**
     * @brief refreshFileInfo 刷新缓存fileinfo
     * @param QUrl 文件的URL
     * @return
     */
    void refreshFileInfo(const QUrl &url);
    /**
     * @brief timeNeedRemoveCache 将需要移除cache的fileurl添加到待移除列表
     * 应用计数小于等于1的
     * @return
     */
    void timeNeedRemoveCache();
    /**
     * @brief timeNeedRemoveCache 将待移除文件列表的文件移除caches
     * @return
     */
    void timeRemoveCache();
};

#endif // DFILEINFOMANAGER_H
