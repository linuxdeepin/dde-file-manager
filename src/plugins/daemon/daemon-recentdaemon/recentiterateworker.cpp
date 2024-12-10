// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
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
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    FinallyUtil finally([this, timestamp]() {
        emit reloadFinished(timestamp);
    });

    QFile file(xbelPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmWarning() << "Failed to open recent file:" << xbelPath;
        return;
    }

    QStringList curPathList;
    const QStringList cachedPathList = itemsInfo.keys();

    QXmlStreamReader reader(&file);
    while (!reader.atEnd() && !reader.hasError()) {
        if (reader.readNext() == QXmlStreamReader::EndDocument)
            continue;

        if (!reader.isStartElement() || reader.name() != "bookmark")
            continue;

        processBookmarkElement(reader, curPathList);
    }

    if (reader.hasError()) {
        fmWarning() << "Error reading recent XML file:" << reader.errorString();
        return;
    }

    removeOutdatedItems(cachedPathList, curPathList);
}

void RecentIterateWorker::processBookmarkElement(QXmlStreamReader &reader, QStringList &curPathList)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());

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
            itemsInfo[bindPath].modified = readTimeSecs;
            emit itemChanged(bindPath, itemsInfo[bindPath]);
        }
    } else {
        RecentItem item { location, readTimeSecs };
        itemsInfo.insert(bindPath, item);
        emit itemAdded(bindPath, item);
    }
}

void RecentIterateWorker::removeOutdatedItems(const QStringList &cachedPathList, const QStringList &curPathList)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());

    QStringList removedPathList;
    for (const auto &cachedPath : cachedPathList) {
        if (!curPathList.contains(cachedPath)) {
            itemsInfo.remove(cachedPath);
            removedPathList << cachedPath;
        }
    }

    if (!removedPathList.isEmpty()) {
        emit itemsRemoved(removedPathList);
    }
}

void RecentIterateWorker::onRequestAddRecentItem(const QVariantMap &item)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    fmDebug() << "Received item:" << item;

    const auto path = item[RecentProperty::kPath].toString();
    const auto appName = item[RecentProperty::kAppName].toString();
    const auto appExec = item[RecentProperty::kAppExec].toString();
    const auto mimeType = item[RecentProperty::kMimeType].toString();

    if (path.isEmpty()) {
        fmWarning() << "add recent item failed, empty path";
        return;
    }

    DTK_CORE_NAMESPACE::DRecentData recentData {
        appName, appExec, mimeType
    };

    if (!DTK_CORE_NAMESPACE::DRecentManager::addItem(path, recentData)) {
        fmWarning() << "add recent item failed: " << path;
        return;
    }

    fmInfo() << "Add " << path << "to recent success";
}

void RecentIterateWorker::onRequestRemoveItems(const QStringList &hrefs)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());

    fmInfo() << "Remove recent items: " << hrefs;
    DTK_CORE_NAMESPACE::DRecentManager::removeItems(hrefs);
}

void RecentIterateWorker::onRequestPurgeItems(const QString &xbelPath)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());

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
        fmInfo() << "Purge recent success: " << xbelPath;
    } else {
        fmWarning() << "purge failed , cannot open recent xbel file !!!";
    }
    emit purgeFinished();
}

SERVERRECENTMANAGER_END_NAMESPACE
