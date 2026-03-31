// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "contentindexupgradeunit.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

using namespace dfm_upgrade;

namespace {

static constexpr char kIndexStatusFileName[] { "index_status.json" };
static constexpr char kOldIndexRelativePath[] { "deepin/dde-file-manager/index" };
static constexpr char kNewIndexRelativePath[] { "deepin/dde-file-manager/fulltext-index" };

}

ContentIndexUpgradeUnit::ContentIndexUpgradeUnit()
    : UpgradeUnit()
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    oldIndexDirPath = QDir(configDir).filePath(kOldIndexRelativePath);
    newIndexDirPath = QDir(dataDir).filePath(kNewIndexRelativePath);

    oldStatusFilePath = QDir(oldIndexDirPath).filePath(kIndexStatusFileName);
    newStatusFilePath = QDir(newIndexDirPath).filePath(kIndexStatusFileName);
}

QString ContentIndexUpgradeUnit::name()
{
    return "ContentIndexUpgradeUnit";
}

bool ContentIndexUpgradeUnit::initialize(const QMap<QString, QString> &args)
{
    Q_UNUSED(args)

    if (!QFileInfo::exists(oldStatusFilePath)) {
        qCInfo(logToolUpgrade) << "ContentIndex: old index status file not found, skip upgrade:" << oldStatusFilePath;
        return false;
    }

    if (QFileInfo::exists(newStatusFilePath)) {
        qCInfo(logToolUpgrade) << "ContentIndex: new index status file already exists, skip upgrade:" << newStatusFilePath;
        return false;
    }

    if (!QDir(oldIndexDirPath).exists()) {
        qCWarning(logToolUpgrade) << "ContentIndex: old index directory missing, skip upgrade:" << oldIndexDirPath;
        return false;
    }

    return true;
}

bool ContentIndexUpgradeUnit::upgrade()
{
    qCInfo(logToolUpgrade) << "ContentIndex: start upgrade from" << oldIndexDirPath << "to" << newIndexDirPath;

    QDir newIndexParentDir(QFileInfo(newIndexDirPath).absolutePath());
    if (!newIndexParentDir.exists() && !newIndexParentDir.mkpath(".")) {
        qCCritical(logToolUpgrade) << "ContentIndex: failed to create new index parent directory:" << newIndexParentDir.path();
        return false;
    }

    if (!canMoveOldIndexDirectory())
        return false;

    QDir dir;
    if (!dir.rename(oldIndexDirPath, newIndexDirPath)) {
        qCCritical(logToolUpgrade) << "ContentIndex: failed to move old index directory from"
                                   << oldIndexDirPath << "to" << newIndexDirPath;
        return false;
    }

    qCInfo(logToolUpgrade) << "ContentIndex: moved old index directory to new location successfully";
    return true;
}

void ContentIndexUpgradeUnit::completed()
{
    qCInfo(logToolUpgrade) << "ContentIndex: upgrade completed";
}

bool ContentIndexUpgradeUnit::canMoveOldIndexDirectory() const
{
    QFileInfo newIndexDirInfo(newIndexDirPath);
    if (!newIndexDirInfo.exists())
        return true;

    if (!newIndexDirInfo.isDir()) {
        qCCritical(logToolUpgrade) << "ContentIndex: target path exists but is not a directory:" << newIndexDirPath;
        return false;
    }

    QDir newIndexDir(newIndexDirPath);
    const QFileInfoList entryInfoList = newIndexDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    if (!entryInfoList.isEmpty()) {
        qCCritical(logToolUpgrade) << "ContentIndex: target directory already contains data, refuse to overwrite:" << newIndexDirPath;
        return false;
    }

    QDir newIndexParentDir(QFileInfo(newIndexDirPath).absolutePath());
    if (!newIndexParentDir.rmdir(QFileInfo(newIndexDirPath).fileName())) {
        qCCritical(logToolUpgrade) << "ContentIndex: failed to remove empty target directory before move:" << newIndexDirPath;
        return false;
    }

    return true;
}
