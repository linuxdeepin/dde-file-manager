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

#include "recentdiriterator.h"
#include "recentutil.h"
#include "recentfileinfo.h"

#include <QQueue>

DPRECENT_BEGIN_NAMESPACE

class RecentDirIteratorPrivate
{
    friend class RecentDirIterator;

public:
    explicit RecentDirIteratorPrivate() {}

private:
    QUrl currentUrl;
    QQueue<QUrl> urlList;
    QMap<QUrl, AbstractFileInfoPointer> recentNodes;
};

RecentDirIterator::RecentDirIterator(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags),
      d(new RecentDirIteratorPrivate())
{
    RecentUtil::initRecentSubSystem();
    for (int i = 0; i < RecentUtil::getRecentNodes().size(); i++) {
        QUrl url = QUrl(RecentUtil::getRecentNodes().at(i).toElement().attribute("href"));
        QUrl schemeUrl = UrlRoute::pathToReal(url.path());
        if (!schemeUrl.isValid())
            continue;
        QFileInfo info(url.toLocalFile());
        if (info.exists() && info.isFile()) {
            d->urlList << schemeUrl;
            d->recentNodes[schemeUrl] = QSharedPointer<RecentFileInfo>(new RecentFileInfo(schemeUrl));
        }
    }
    // ToDo(yanghao):watcher
}

RecentDirIterator::~RecentDirIterator()
{
    if (d) {
        delete d;
    }
}

QUrl RecentDirIterator::next()
{
    if (!d->urlList.isEmpty()) {
        d->currentUrl = d->urlList.dequeue();
        return d->currentUrl;
    }

    return QUrl();
}

bool RecentDirIterator::hasNext() const
{
    if (!d->urlList.isEmpty()) {
        return true;
    }

    return false;
}

QString RecentDirIterator::fileName() const
{
    AbstractFileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->fileName() : QString();
}

QUrl RecentDirIterator::fileUrl() const
{
    AbstractFileInfoPointer currentInfo = d->recentNodes.value(d->currentUrl);

    return currentInfo ? currentInfo->url() : QString();
}

const AbstractFileInfoPointer RecentDirIterator::fileInfo() const
{
    return d->recentNodes.value(d->currentUrl);
}

QUrl RecentDirIterator::url() const
{
    return RecentUtil::rootUrl();
}

DPRECENT_END_NAMESPACE
