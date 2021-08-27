/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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

#ifndef DFMINFOCACHESMANAGER_P_H
#define DFMINFOCACHESMANAGER_P_H

#include "dfminfocache.h"

#include <QMutex>
#include <QTimer>

class DFMInfoCachePrivate
{
    Q_DECLARE_PUBLIC(DFMInfoCache)
    DFMInfoCache *q_ptr;

public:
    //缓存fileifno的Map
    DThreadMap<QUrl, DAbstractFileInfoPointer> m_fileInfos;
    //待移除的fileinfo的urllist
    DThreadList<QUrl> m_needRemoveCacheList;
    //已被removecache的url
    DThreadList<QUrl> m_removedCacheList;
    //已被SortByTimeCache的url
    DThreadList<QUrl> m_removedSortByTimeCacheList;
    //刷新线程
    QSharedPointer<ReFreshThread> m_refreshThread = nullptr;
    //按时间排序的缓存fileinfo的文件url
    DThreadList<QUrl> m_sortByTimeCacheUrl;
    //需要加入待移除缓存的计时器
    QTimer m_needRemoveTimer;
    //移除缓存的
    QTimer m_removeTimer;

    explicit DFMInfoCachePrivate(DFMInfoCache *qq);

    virtual ~DFMInfoCachePrivate();

    bool canReleaseFile(const QUrl &QUrl);
    /**
     * @brief updateInfoTimeList 根据URL跟新按时间排序的链表
     * 移除更新时间信息和待更新信息，一般是在移除fileinfo，cache析构的时候
     * @param QUrl 文件的URL
     * @return
     */
    void updateSortByTimeCacheUrlList(const QUrl &url);
    /**
     * @brief removeNeedReleaseList 根据URL从待removecache的文件列表中移除
     * @param QUrl 文件的URL
     * @return
     */
};

#endif // DFMINFOCACHESMANAGER_P_H
