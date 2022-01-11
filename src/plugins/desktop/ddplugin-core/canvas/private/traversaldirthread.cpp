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

#include "traversaldirthread.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"

DSB_D_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

dfm_service_desktop::TraversalDirThread::TraversalDirThread(const QUrl &url
                                                            , const QStringList &nameFilters
                                                            , QDir::Filters filters
                                                            , QDirIterator::IteratorFlags flags
                                                            , QObject *parent)
    : QThread (parent)
    , dirUrl(url)
{
    if (dirUrl.isValid() && !UrlRoute::isVirtual(dirUrl)) {
        dirIterator = DirIteratorFactory::create<LocalDirIterator>(url, nameFilters, filters, flags);
        if (!dirIterator) {
            qInfo() << "Failed create dir iterator from" << url;
            abort();
        }
    }
}

dfm_service_desktop::TraversalDirThread::~TraversalDirThread()
{

}

void dfm_service_desktop::TraversalDirThread::stop()
{
    stopFlag = true;
}

void dfm_service_desktop::TraversalDirThread::quit()
{
    stop();
    QThread::quit();
}

void dfm_service_desktop::TraversalDirThread::run()
{
    if (dirIterator.isNull())
        return;

    while (dirIterator->hasNext()) {
        if (stopFlag)
            break;

        QUrl fileurl = dirIterator->next();
        if (!fileurl.isValid())
            continue;

        fileUrls.append(fileurl);
    }
    stopFlag = true;
    Q_EMIT updateChildren(fileUrls.list());
}
