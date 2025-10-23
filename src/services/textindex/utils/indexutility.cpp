// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "indexutility.h"
#include "textindexconfig.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>

inline constexpr char kDeepinAnythingDconfName[] { "org.deepin.anything" };
inline constexpr char kDeepinAnythingDconfPathKey[] { "indexing_paths" };

DCORE_USE_NAMESPACE
SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace IndexUtility {

bool isIndexWithAnything(const QString &path)
{
    if (!DFMSEARCH::Global::isFileNameIndexReadyForSearch())
        return false;

    return isDefaultIndexedDirectory(path);
}

bool isDefaultIndexedDirectory(const QString &path)
{
    auto kDirs = AnythingConfigWatcher::instance()->defaultAnythingIndexPaths();
    return kDirs.contains(path);
}

bool isPathInContentIndexDirectory(const QString &path)
{
    if (!DFMSEARCH::Global::isContentIndexAvailable())
        return false;

    static const QStringList &kDirs = DFMSEARCH::Global::defaultIndexedDirectory();
    return std::any_of(kDirs.cbegin(), kDirs.cend(),
                       [&path](const QString &dir) {
                           // Normalize both paths by ensuring they don't end with '/'
                           QString normalizedDir = dir;
                           QString normalizedPath = path;

                           if (normalizedDir.endsWith('/') && normalizedDir.length() > 1) {
                               normalizedDir.chop(1);
                           }
                           if (normalizedPath.endsWith('/') && normalizedPath.length() > 1) {
                               normalizedPath.chop(1);
                           }

                           // Exact match - the path is the indexed directory itself
                           if (normalizedPath == normalizedDir) {
                               return true;
                           }

                           // Check if path is within the directory by ensuring proper path separation
                           // The path must start with the directory + '/' to avoid false positives
                           // like '/foobar' matching '/foo'
                           const QString dirWithSeparator = normalizedDir + '/';
                           return normalizedPath.startsWith(dirWithSeparator);
                       });
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
        const QStringList &files = dir.entryList(QDir::Files);
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

bool checkFileSize(const QFileInfo &fileInfo)
{
    try {
        // Get the max file size from config each time to support testing
        qint64 sizeMBFromConfig = TextIndexConfig::instance().maxIndexFileSizeMB();
        // 在这里进行上述的健全性检查
        if (sizeMBFromConfig <= 0 || sizeMBFromConfig > Q_INT64_C(0x7FFFFFFFFFFFFFFF) / (1024LL * 1024LL)) {
            sizeMBFromConfig = 50LL;   // Default fallback
        }
        const qint64 kMaxFileSizeInBytes = sizeMBFromConfig * 1024LL * 1024LL;

        if (fileInfo.size() > kMaxFileSizeInBytes) {
            fmDebug() << "File" << fileInfo.fileName() << "size" << fileInfo.size()
                      << "exceeds max allowed size" << kMaxFileSizeInBytes;
            return false;
        }
        return true;
    } catch (const std::exception &e) {
        fmWarning() << "Failed to check file size:" << fileInfo.filePath() << e.what();
        return false;
    } catch (...) {
        fmWarning() << "Failed to check file size with unknown exception:" << fileInfo.filePath();
        return false;
    }
}

bool isSupportedFile(const QString &path)
{
    try {
        QFileInfo fileInfo(path);
        if (!fileInfo.exists() || !fileInfo.isFile())
            return false;

        // 检查文件大小是否超过 X MB（X * 1024 * 1024 字节）
        if (!checkFileSize(fileInfo))
            return false;

        const QString &suffix = fileInfo.suffix().toLower();
        return TextIndexConfig::instance().supportedFileExtensions().contains(suffix);
    } catch (const std::exception &e) {
        fmWarning() << "Failed to check if file is supported:" << path << e.what();
        return false;
    } catch (...) {
        fmWarning() << "Failed to check if file is supported with unknown exception:" << path;
        return false;
    }
}

AnythingConfigWatcher *AnythingConfigWatcher::instance()
{
    static AnythingConfigWatcher *in = new AnythingConfigWatcher;
    return in;
}

AnythingConfigWatcher::~AnythingConfigWatcher()
{
}

QStringList AnythingConfigWatcher::defaultAnythingIndexPaths()
{
    QMutexLocker lk(&mu);
    return defaultIndexPath;
}

QStringList AnythingConfigWatcher::defaultAnythingIndexPathsRealtime()
{
    QMutexLocker lk(&mu);
    defaultIndexPath.clear();
    defaultIndexPath = DFMSEARCH::Global::defaultIndexedDirectory();
    return defaultIndexPath;
}

void AnythingConfigWatcher::handleConfigChanged(const QString &key)
{
    if (key != kDeepinAnythingDconfPathKey)
        return;
    defaultAnythingIndexPathsRealtime();
}

AnythingConfigWatcher::AnythingConfigWatcher(QObject *parent)
    : QObject(parent)
{
    cfg = DConfig::create(kDeepinAnythingDconfName, kDeepinAnythingDconfName, "", this);
    if (!cfg)
        qWarning() << " [AnythingConfigWatcher::AnythingConfigWatcher] create dconfig error, nullptr!!" << kDeepinAnythingDconfName;

    if (cfg && !cfg->isValid()) {
        qWarning() << " [AnythingConfigWatcher::AnythingConfigWatcher] create dconfig error, config is not valid!!" << kDeepinAnythingDconfName;
        cfg->deleteLater();
        cfg = nullptr;
    }

    if (cfg && cfg->isValid())
        connect(cfg, &DConfig::valueChanged, this, &AnythingConfigWatcher::handleConfigChanged);

    defaultAnythingIndexPathsRealtime();
}

}   // namespace IndexUtility

namespace PathCalculator {

QString calculateNewPathForDirectoryMove(const QString &oldPath,
                                         const QString &fromDirPath,
                                         const QString &toDirPath)
{
    if (oldPath.startsWith(fromDirPath)) {
        return toDirPath + "/" + oldPath.mid(fromDirPath.length());
    } else if (oldPath == fromDirPath.chopped(1)) {   // Remove trailing slash for comparison
        return toDirPath;
    }
    return oldPath;   // No change needed
}

QString normalizeDirectoryPath(const QString &dirPath)
{
    QString normalized = dirPath;
    if (!normalized.endsWith('/')) {
        normalized += '/';
    }
    return normalized;
}

bool isDirectoryMove(const QString &toPath)
{
    if (toPath.isEmpty()) {
        return false;
    }

    // First check if path exists and is a directory
    QFileInfo toFileInfo(toPath);
    if (toFileInfo.exists()) {
        return toFileInfo.isDir();
    }

    // If path doesn't exist, infer from path format (trailing slash indicates directory)
    return toPath.endsWith('/');
}

}   // namespace PathCalculator

SERVICETEXTINDEX_END_NAMESPACE
