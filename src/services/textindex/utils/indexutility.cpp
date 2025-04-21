// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "indexutility.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace IndexUtility {

bool isIndexWithAnything(const QString &path)
{
    if (!DFMSEARCH::Global::isFileNameIndexDirectoryAvailable())
        return false;

    return isDefaultIndexedDirectory(path);
}

bool isDefaultIndexedDirectory(const QString &path)
{
    return DFMSEARCH::Global::defaultIndexedDirectory().contains(path);
}

bool isPathInContentIndexDirectory(const QString &path)
{
    return DFMSEARCH::Global::isPathInContentIndexDirectory(path);
}

QString statusFilePath()
{
    return DFMSEARCH::Global::contentIndexDirectory() + "/index_status.json";
}

void removeIndexStatusFile()
{
    QFile file(statusFilePath());
    if (file.exists()) {
        fmInfo() << "Clearing index status file:" << file.fileName()
                 << "[Clearing index status configuration]";
        file.remove();
    }
}

void clearIndexDirectory()
{
    QString indexDir = DFMSEARCH::Global::contentIndexDirectory();
    QDir dir(indexDir);

    if (dir.exists()) {
        // 删除所有索引文件
        QStringList files = dir.entryList(QDir::Files);
        for (const QString &file : files) {
            if (dir.remove(file)) {
                fmWarning() << "Removed corrupted index file:" << file;
            } else {
                fmWarning() << "Failed to remove index file:" << file;
            }
        }
    }

    // 确保目录存在
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void saveIndexStatus(const QDateTime &lastUpdateTime)
{
    saveIndexStatus(lastUpdateTime, Defines::kIndexVersion);
}

void saveIndexStatus(const QDateTime &lastUpdateTime, int version)
{
    QJsonObject status;
    status[Defines::kLastUpdateTimeKey] = lastUpdateTime.toString(Qt::ISODate);
    status[Defines::kVersionKey] = version;

    QJsonDocument doc(status);
    QFile file(statusFilePath());

    // 确保目录存在
    QDir().mkpath(QFileInfo(file).absolutePath());

    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        fmInfo() << "Index status saved successfully:" << file.fileName()
                 << "lastUpdateTime:" << lastUpdateTime.toString(Qt::ISODate)
                 << "version:" << version
                 << "[Updated index status configuration]";
    } else {
        fmWarning() << "Failed to save index status to:" << file.fileName()
                    << "[Failed to write index status configuration]";
    }
}

QString getLastUpdateTime()
{
    QFile file(IndexUtility::statusFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains(Defines::kLastUpdateTimeKey)) {
            QDateTime time = QDateTime::fromString(obj[Defines::kLastUpdateTimeKey].toString(), Qt::ISODate);
            return time.toString("yyyy-MM-dd hh:mm:ss");
        }
    }
    return QString();
}

int getIndexVersion()
{
    QFile file(IndexUtility::statusFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;   // File doesn't exist or can't be opened
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains(Defines::kVersionKey)) {
            return obj[Defines::kVersionKey].toInt(-1);
        }
    }
    return -1;   // Version not found in status file
}

bool isCompatibleVersion()
{
    int currentVersion = getIndexVersion();

    // If version is -1, it means either:
    // 1. Status file doesn't exist
    // 2. Status file is corrupted
    // 3. Version field is not present in the status file
    // In all these cases, we need to rebuild the index
    if (currentVersion == -1) {
        fmWarning() << "Index version not found or invalid in status file"
                    << "[Index compatibility check failed]";
        return false;
    }

    // Check if the version in status file matches the current code version
    bool isCompatible = (currentVersion == Defines::kIndexVersion);
    if (!isCompatible) {
        fmWarning() << "Index version mismatch. Status file version:" << currentVersion
                    << "Current code version:" << Defines::kIndexVersion
                    << "[Index version incompatible]";
    }

    return isCompatible;
}

}   // namespace IndexUtility

SERVICETEXTINDEX_END_NAMESPACE
