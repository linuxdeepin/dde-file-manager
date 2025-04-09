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

bool isDefaultIndexedDirectory(const QString &path)
{
    return DFMSEARCH::Global::defaultIndexedDirectory().contains(path);
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
    QJsonObject status;
    status[Defines::kLastUpdateTime] = lastUpdateTime.toString(Qt::ISODate);

    QJsonDocument doc(status);
    QFile file(statusFilePath());

    // 确保目录存在
    QDir().mkpath(QFileInfo(file).absolutePath());

    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        fmInfo() << "Index status saved successfully:" << file.fileName()
                 << "lastUpdateTime:" << lastUpdateTime.toString(Qt::ISODate)
                 << "[Updated index status configuration]";
    } else {
        fmWarning() << "Failed to save index status to:" << file.fileName()
                    << "[Failed to write index status configuration]";
    }
}

}   // namespace IndexUtility

SERVICETEXTINDEX_END_NAMESPACE
