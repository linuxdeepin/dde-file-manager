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
#ifndef TRAVERSALDIRTHREAD_H
#define TRAVERSALDIRTHREAD_H

#include "dfm-base/dfm_base_global.h"
#include "localfile/localdiriterator.h"
#include "widgets/dfmfileview/fileviewitem.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QMetaType>
#include <QObject>
#include <QThread>
#include <QUrl>

DFMBASE_BEGIN_NAMESPACE

class TraversalDirThread : public QThread
{
    Q_OBJECT
    QUrl dirUrl; // 遍历的目录的url
    QSharedPointer<LocalDirIterator> m_dirIterator; // 当前遍历目录的diriterator
    DThreadList<FileViewItem*> m_childrenList; // 当前遍历出来的所有文件
    bool stopFlag = false;

public:
    explicit TraversalDirThread(const QUrl& url, const QStringList &nameFilters = QStringList(),
                                   QDir::Filters filters = QDir::NoFilter,
                                   QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                   QObject *parent = nullptr);
    virtual ~TraversalDirThread() override;
    void stop();
    void quit();

Q_SIGNALS:
    void updateChildren(const QList<FileViewItem*> &children);
    void stoped();

protected:
    virtual void run() override;
};

DFMBASE_END_NAMESPACE

#endif // DFMTRAVERSALDIRTHREAD_H
