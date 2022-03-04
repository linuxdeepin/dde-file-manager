/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef TRAVERSALDIRTHREAD_H
#define TRAVERSALDIRTHREAD_H

#include "dfm_base_global.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QMetaType>
#include <QObject>
#include <QThread>
#include <QUrl>

DFMBASE_BEGIN_NAMESPACE

class TraversalDirThread : public QThread
{
    Q_OBJECT
    QUrl dirUrl;   // 遍历的目录的url
    QSharedPointer<dfmbase::AbstractDirIterator> dirIterator;   // 当前遍历目录的diriterator
    QList<QUrl> childrenList;   // 当前遍历出来的所有文件
    bool stopFlag = false;

public:
    explicit TraversalDirThread(const QUrl &url, const QStringList &nameFilters = QStringList(),
                                dfmio::DEnumerator::DirFilters filters = dfmio::DEnumerator::DirFilter::NoFilter,
                                dfmio::DEnumerator::IteratorFlags flags = dfmio::DEnumerator::IteratorFlag::NoIteratorFlags,
                                QObject *parent = nullptr);
    virtual ~TraversalDirThread() override;
    void stop();
    void quit();
    void stopAndDeleteLater();

Q_SIGNALS:
    void updateChildren(const QList<QUrl> children);
    void updateChild(const QUrl url);
    void stoped();

protected:
    virtual void run() override;
};

DFMBASE_END_NAMESPACE

#endif   // DFMTRAVERSALDIRTHREAD_H
