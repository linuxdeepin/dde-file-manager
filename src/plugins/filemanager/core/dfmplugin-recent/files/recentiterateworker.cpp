// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentiterateworker.h"
#include "utils/recentfilehelper.h"
#include "utils/recentmanager.h"
#include "files/recentfileinfo.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

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

    QFile file(RecentHelper::xbelPath());
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
                auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
                if (info && info->exists() && info->isAttributes(OptInfoType::kIsFile)) {
                    const auto &bindPath = FileUtils::bindPathTransform(info->pathOf(PathInfoType::kAbsoluteFilePath), false);
                    QUrl recentUrl = QUrl::fromLocalFile(bindPath);
                    recentUrl.setScheme(RecentHelper::scheme());
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
