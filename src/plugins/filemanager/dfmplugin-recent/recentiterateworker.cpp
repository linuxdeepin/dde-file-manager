/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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

#include "recentiterateworker.h"
#include "utils/recenthelper.h"

#include "dfm-base/base/schemefactory.h"

#include <QDir>
#include <QXmlStreamReader>
#include <QUrl>
#include <QMetaType>
#include <QList>

DFMBASE_USE_NAMESPACE
DPRECENT_BEGIN_NAMESPACE

RecentIterateWorker::RecentIterateWorker()
    : QObject()
{
    qRegisterMetaType<QList<QPair<QUrl, qint64>>>("QList<QPair<QUrl,qint64> >&");
}

void RecentIterateWorker::doWork()
{
    QFile file(RecentHelper::xbelPath());
    QList<QPair<QUrl, qint64>> urlList;

    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);

        while (!reader.atEnd()) {

            if (!reader.readNextStartElement() || reader.name() != "bookmark") {
                continue;
            }

            const QStringRef &location = reader.attributes().value("href");
            const QStringRef &readTime = reader.attributes().value("modified");

            if (!location.isEmpty()) {
                QUrl url = QUrl(location.toString());
                QFileInfo info(url.toLocalFile());
                QUrl recentUrl = url;
                recentUrl.setScheme(RecentHelper::scheme());
                if (info.exists() && info.isFile()) {
                    qint64 readTimeSecs = QDateTime::fromString(readTime.toString(), Qt::ISODate).toSecsSinceEpoch();
                    urlList.append(qMakePair(recentUrl, readTimeSecs));
                }
            }
        }
        emit recentUrls(urlList);
    }
}
DPRECENT_END_NAMESPACE
