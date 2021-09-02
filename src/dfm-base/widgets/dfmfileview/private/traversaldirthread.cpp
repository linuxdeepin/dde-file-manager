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
#include "traversaldirthread.h"
#include "base/schemefactory.h"

DFMTraversalDirThread::DFMTraversalDirThread(const QUrl &url,
                                             const QStringList &nameFilters,
                                             QDir::Filters filters,
                                             QDirIterator::IteratorFlags flags,
                                             QObject *parent)
    : QThread(parent)
    , m_dirUrl(url)
{
    if (m_dirUrl.isValid()) {
        m_dirIterator = DirIteratorFactory::instance().create<LocalDirIterator>
                            (url, nameFilters, filters,flags);
    }
    qRegisterMetaType<QList<QSharedPointer<DFMFileViewItem>>>("QList<QSharedPointer<DFMFileViewItem>>");
    qRegisterMetaType<QList<QSharedPointer<DFMFileViewItem>>>("QList<QSharedPointer<DFMFileViewItem>>&");
}

DFMTraversalDirThread::~DFMTraversalDirThread()
{

}


void DFMTraversalDirThread::run()
{
    if (m_dirIterator.isNull())
        return;
    while(m_dirIterator->hasNext())
    {
        m_dirIterator->next();
        QUrl fileurl = m_dirIterator->fileUrl();
        if (!fileurl.isValid()) continue;
        QSharedPointer<DFMFileViewItem> item(new DFMFileViewItem(fileurl));
        m_childrenList.append(item);
    }
    Q_EMIT updateChildren(m_childrenList.list());
}
