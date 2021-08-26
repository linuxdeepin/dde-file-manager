/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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
#ifndef DFMTRAVERSALDIRTHREAD_H
#define DFMTRAVERSALDIRTHREAD_H

#include "localfile/dfmlocaldiriterator.h"
#include "base/private/dfmthreadcontainer.hpp"
#include "widgets/dfmfileview/dfmfileviewitem.h"

#include <QMetaType>
#include <QObject>
#include <QThread>
#include <QUrl>

class DFMTraversalDirThread : public QThread
{
    Q_OBJECT
public:
    explicit DFMTraversalDirThread(const QUrl& url, const QStringList &nameFilters = QStringList(),
                                   QDir::Filters filters = QDir::NoFilter,
                                   QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                   QObject *parent = nullptr);
    ~DFMTraversalDirThread() override;

Q_SIGNALS:
    void updateChildren(const QList<QSharedPointer<DFMFileViewItem>> &children);

    // QThread interface
protected:
    virtual void run() override;
private:
    //遍历的目录的url
    QUrl m_dirUrl;
    //当前遍历目录的diriterator
    QSharedPointer<DFMLocalDirIterator> m_dirIterator;
    //当前遍历出来的所有文件
    DThreadList<QSharedPointer<DFMFileViewItem>> m_childrenList;
};

#endif // DFMTRAVERSALDIRTHREAD_H
