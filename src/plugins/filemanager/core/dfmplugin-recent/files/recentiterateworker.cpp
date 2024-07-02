// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentiterateworker.h"
#include "utils/recentfilehelper.h"
#include "utils/recentmanager.h"
#include "files/recentfileinfo.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/device/deviceutils.h>

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

void RecentIterateWorker::onRecentFileChanged(const QList<QUrl> &cachedUrls)
{
    QFile file(RecentHelper::xbelPath());
    QList<QUrl> urlList;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::EndDocument)
            continue;

        if (!reader.isStartElement() || reader.name() != "bookmark")
            continue;

        const QString &location = reader.attributes().value("href").toString();
        const QString &readTime = reader.attributes().value("modified").toString();

        if (location.isEmpty())
            continue;

        if (stopped)
            return;

        const QUrl &url { QUrl(location) };
        if (DeviceUtils::isLowSpeedDevice(url))
            continue;

        auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
        if (stopped)
            return;
        if (info && info->exists() && info->isAttributes(OptInfoType::kIsFile)) {
            const auto &bindPath = FileUtils::bindPathTransform(info->pathOf(PathInfoType::kAbsoluteFilePath), false);
            QUrl recentUrl { QUrl::fromLocalFile(bindPath) };
            recentUrl.setScheme(RecentHelper::scheme());
            qint64 readTimeSecs = QDateTime::fromString(readTime, Qt::ISODate).toSecsSinceEpoch();
            urlList.append(recentUrl);
            emit updateRecentFileInfo(recentUrl, location, readTimeSecs);
        }
    }

    if (reader.hasError()) {
        fmWarning() << "Read recent xml file has error! Error: " << reader.errorString();
        return;
    }

    // delete cached recent file when recent file removed
    QList<QUrl> deletedUrls;
    for (const QUrl &url : cachedUrls) {
        if (!urlList.contains(url))
            deletedUrls << url;
    }
    if (!deletedUrls.isEmpty())
        emit deleteExistRecentUrls(deletedUrls);
}

void RecentIterateWorker::stop()
{
    stopped = true;
}

}   // namespace dfmplugin_recent
