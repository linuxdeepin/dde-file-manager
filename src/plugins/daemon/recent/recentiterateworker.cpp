// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentiterateworker.h"

#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/finallyutil.h>
#include <dfm-base/utils/protocolutils.h>

#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QUrl>

SERVERRECENTMANAGER_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

RecentIterateWorker::RecentIterateWorker(QObject *parent)
    : QObject(parent)
{
}

// 对 xbel 的增删改都会触发本函数重新扫描 xbel 文件
void RecentIterateWorker::onRequestReload(const QString &xbelPath, qint64 timestamp)
{
    // Q_ASSERT(qApp->thread() != QThread::currentThread());
    FinallyUtil finally([this, timestamp]() {
        emit reloadFinished(timestamp);
    });

    QFile file(xbelPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmCritical() << "[RecentIterateWorker::onRequestReload] Failed to open recent file:" << xbelPath;
        return;
    }
    fmDebug() << "[RecentIterateWorker::onRequestReload] Successfully opened recent file:" << xbelPath;

    QStringList curPathList;
    const QStringList cachedPathList = itemsInfo.keys();

    QXmlStreamReader reader(&file);
    while (!reader.atEnd() && !reader.hasError()) {
        if (reader.readNext() == QXmlStreamReader::EndDocument)
            continue;

        if (!reader.isStartElement() || reader.name() != QString("bookmark"))
            continue;

        processBookmarkElement(reader, curPathList);
    }

    if (reader.hasError()) {
        fmCritical() << "[RecentIterateWorker::onRequestReload] Error reading recent XML file:" << xbelPath
                     << "error:" << reader.errorString();
        return;
    }

    fmInfo() << "[RecentIterateWorker::onRequestReload] Successfully processed recent file:" << xbelPath
             << "current items:" << curPathList.size() << "cached items:" << cachedPathList.size();

    removeOutdatedItems(cachedPathList, curPathList);
}

void RecentIterateWorker::processBookmarkElement(QXmlStreamReader &reader, QStringList &curPathList)
{
    // Q_ASSERT(qApp->thread() != QThread::currentThread());

    const QString location = reader.attributes().value("href").toString();
    const QString readTime = reader.attributes().value("modified").toString();

    if (location.isEmpty())
        return;

    const QUrl url(location);
    if (!url.isLocalFile())
        return;
    if (ProtocolUtils::isRemoteFile(url))
        return;

    QFileInfo info(url.toLocalFile());
    if (!info.exists() || !info.isFile())
        return;

    const auto bindPath = FileUtils::bindPathTransform(info.absoluteFilePath(), false);
    qint64 readTimeSecs = QDateTime::fromString(readTime, Qt::ISODate).toSecsSinceEpoch();

    curPathList.append(bindPath);
    if (itemsInfo.contains(bindPath)) {
        if (itemsInfo[bindPath].modified != readTimeSecs) {
            fmDebug() << "[RecentIterateWorker::processBookmarkElement] Item modified:" << bindPath
                      << "old time:" << itemsInfo[bindPath].modified << "new time:" << readTimeSecs;
            itemsInfo[bindPath].modified = readTimeSecs;
            emit itemChanged(bindPath, itemsInfo[bindPath]);
        }
    } else {
        fmDebug() << "[RecentIterateWorker::processBookmarkElement] New item added:" << bindPath
                  << "modified time:" << readTimeSecs;
        RecentItem item { location, readTimeSecs };
        itemsInfo.insert(bindPath, item);
        emit itemAdded(bindPath, item);
    }
}

void RecentIterateWorker::removeOutdatedItems(const QStringList &cachedPathList, const QStringList &curPathList)
{
    // Q_ASSERT(qApp->thread() != QThread::currentThread());

    QStringList removedPathList;
    for (const auto &cachedPath : cachedPathList) {
        if (!curPathList.contains(cachedPath)) {
            itemsInfo.remove(cachedPath);
            removedPathList << cachedPath;
        }
    }

    if (!removedPathList.isEmpty()) {
        fmInfo() << "[RecentIterateWorker::removeOutdatedItems] Removed outdated items:" << removedPathList.size();
        emit itemsRemoved(removedPathList);
    }
}

void RecentIterateWorker::onRequestAddRecentItem(const QVariantMap &item)
{
    // Q_ASSERT(qApp->thread() != QThread::currentThread());
    fmDebug() << "[RecentIterateWorker::onRequestAddRecentItem] Received item request:" << item;

    const auto path = item[RecentProperty::kPath].toString();
    const auto appName = item[RecentProperty::kAppName].toString();
    const auto appExec = item[RecentProperty::kAppExec].toString();
    const auto mimeType = item[RecentProperty::kMimeType].toString();

    if (path.isEmpty()) {
        fmWarning() << "[RecentIterateWorker::onRequestAddRecentItem] Failed to add recent item: empty path provided";
        return;
    }

    DTK_CORE_NAMESPACE::DRecentData recentData {
        appName, appExec, mimeType
    };

    if (!DTK_CORE_NAMESPACE::DRecentManager::addItem(path, recentData)) {
        fmCritical() << "[RecentIterateWorker::onRequestAddRecentItem] Failed to add recent item:" << path
                     << "app:" << appName << "mime:" << mimeType;
        return;
    }

    fmInfo() << "[RecentIterateWorker::onRequestAddRecentItem] Successfully added recent item:" << path
             << "app:" << appName;
}

void RecentIterateWorker::onRequestRemoveItems(const QStringList &hrefs)
{
    //   Q_ASSERT(qApp->thread() != QThread::currentThread());

    fmInfo() << "[RecentIterateWorker::onRequestRemoveItems] Removing recent items:" << hrefs.size() << "items";
    DTK_CORE_NAMESPACE::DRecentManager::removeItems(hrefs);
    fmInfo() << "[RecentIterateWorker::onRequestRemoveItems] Successfully removed recent items";
}

void RecentIterateWorker::onRequestPurgeItems(const QString &xbelPath)
{
    //   Q_ASSERT(qApp->thread() != QThread::currentThread());

    static constexpr char kEmptyRecentFile[] =
            R"|(<?xml version="1.0" encoding="UTF-8"?>
            <xbel version="1.0"
            xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks"
            xmlns:mime="http://www.freedesktop.org/standards/shared-mime-info"
            >
            </xbel>)|";

    QFile file(xbelPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(kEmptyRecentFile);
        file.close();
        fmInfo() << "[RecentIterateWorker::onRequestPurgeItems] Successfully purged recent file:" << xbelPath;
    } else {
        fmCritical() << "[RecentIterateWorker::onRequestPurgeItems] Failed to purge recent file, cannot open:" << xbelPath;
    }
    emit purgeFinished();
}

SERVERRECENTMANAGER_END_NAMESPACE
