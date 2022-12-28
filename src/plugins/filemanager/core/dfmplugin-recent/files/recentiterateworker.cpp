/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "utils/recentmanager.h"
#include "files/recentfileinfo.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/fileutils.h"

#include <QDir>
#include <QXmlStreamReader>
#include <QUrl>
#include <QMetaType>
#include <QList>
#include <QMutexLocker>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_recent {

RecentIterateWorker::RecentIterateWorker()
    : QObject()
{
}

void RecentIterateWorker::doWork()
{

    QFile file(RecentManager::xbelPath());
    QList<QUrl> urlList;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QXmlStreamReader reader(&file);

        while (!reader.atEnd()) {

            if (!reader.readNextStartElement() || reader.name() != "bookmark") {
                continue;
            }

            const QStringRef &location = reader.attributes().value("href");
            const QStringRef &readTime = reader.attributes().value("modified");

            if (!location.isEmpty()) {
                QUrl url = QUrl(location.toString());
                auto info = InfoFactory::create<AbstractFileInfo>(url);
                if (info && info->exists() && info->isAttributes(OptInfoType::kIsFile)) {
                    const auto &bindPath = FileUtils::bindPathTransform(info->pathOf(PathInfoType::kAbsoluteFilePath), false);
                    QUrl recentUrl = QUrl::fromLocalFile(bindPath);
                    recentUrl.setScheme(RecentManager::scheme());
                    qint64 readTimeSecs = QDateTime::fromString(readTime.toString(), Qt::ISODate).toSecsSinceEpoch();
                    urlList.append(recentUrl);
                    emit updateRecentFileInfo(recentUrl, location.toString(), readTimeSecs);
                }
            }
        }
        file.close();

        QList<QUrl> deleteUrls;
        const auto &recentNodes = RecentManager::instance()->getRecentNodes();
        for (const QUrl &url : recentNodes.keys()) {
            if (!urlList.contains(url)) {
                deleteUrls << url;
            } else {
                auto info = recentNodes[url];
                if (!info) {
                    deleteUrls << url;
                }
            }
        }
        if (!deleteUrls.isEmpty())
            emit deleteExistRecentUrls(deleteUrls);
    }
}
}
