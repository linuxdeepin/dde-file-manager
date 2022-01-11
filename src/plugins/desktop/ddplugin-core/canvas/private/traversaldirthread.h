/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "dfm_desktop_service_global.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "dfm-base/utils/threadcontainer.hpp"

#include <QThread>
#include <QUrl>

DSB_D_BEGIN_NAMESPACE
class TraversalDirThread : public QThread
{
    Q_OBJECT
public:
    explicit TraversalDirThread(const QUrl &url, const QStringList &nameFilters = QStringList(),
                                QDir::Filters filters = QDir::NoFilter,
                                QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags,
                                QObject *parent = nullptr);
    virtual ~TraversalDirThread() override;

    void stop();
    void quit();

Q_SIGNALS:
    void updateChildren(const QList<QUrl> &files);
    void stoped();

protected:
    virtual void run() override;

private:
    QUrl dirUrl;
    QSharedPointer<dfmbase::LocalDirIterator> dirIterator;
    dfmbase::DThreadList<QUrl> fileUrls;
    bool stopFlag = false;
};

DSB_D_END_NAMESPACE
#endif // TRAVERSALDIRTHREAD_H
