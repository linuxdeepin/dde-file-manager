// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "indexutility.h"
#include "textindexconfig.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>
#include <QSaveFile>

inline constexpr char kDeepinAnythingDconfName[] { "org.deepin.anything" };
inline constexpr char kDeepinAnythingDconfPathKey[] { "indexing_paths" };
inline constexpr char kDeepinAnythingDconfBlacklistPathKey[] { "blacklist_paths" };

DCORE_USE_NAMESPACE
SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace IndexUtility {

bool isIndexWithAnything(const QString &path)
{
    auto status = DFMSEARCH::Global::fileNameIndexStatus();
    if (!status.has_value()) {
        fmWarning() << "Anything indexing is disabled";
        return {};
    }

    const QString currentStatus = status.value();
    if (currentStatus == "closed") {
        fmWarning() << "Anything indexing is closed";
        return {};
    }

    return isDefaultIndexedDirectory(path);
}

bool isDefaultIndexedDirectory(const QString &path)
{
    auto kDirs = AnythingConfigWatcher::instance()->defaultAnythingIndexPaths();
    return kDirs.contains(path);
}

bool checkFileSize(const QFileInfo &fileInfo, qint64 sizeMBFromConfig)
{
    try {
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

bool isSupportedTextFile(const QString &path)
{
    try {
        QFileInfo fileInfo(path);

        // 检查文件大小是否超过 X MB（X * 1024 * 1024 字节）
        if (fileInfo.exists() && !checkFileSize(fileInfo, TextIndexConfig::instance().maxIndexTextFileSizeMB()))
            return false;

        const QString &suffix = fileInfo.suffix().toLower();
        return TextIndexConfig::instance().supportedTextFileExtensions().contains(suffix);
    } catch (const std::exception &e) {
        fmWarning() << "Failed to check if file is supported:" << path << e.what();
        return false;
    } catch (...) {
        fmWarning() << "Failed to check if file is supported with unknown exception:" << path;
        return false;
    }
}

bool isSupportedOCRFile(const QString &path)
{
    try {
        QFileInfo fileInfo(path);

        // 检查文件大小是否超过 X MB（X * 1024 * 1024 字节）
        if (fileInfo.exists() && !checkFileSize(fileInfo, TextIndexConfig::instance().maxOcrImageSizeMB()))
            return false;

        const QString &suffix = fileInfo.suffix().toLower();
        return TextIndexConfig::instance().supportedOcrImageExtensions().contains(suffix);
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

QStringList AnythingConfigWatcher::defaultBlacklistPaths()
{
    QMutexLocker lk(&mu);
    return blacklistPaths;
}

QStringList AnythingConfigWatcher::defaultBlacklistPathsRealtime()
{
    QMutexLocker lk(&mu);
    blacklistPaths.clear();
    blacklistPaths = DFMSEARCH::Global::defaultBlacklistPaths();
    return blacklistPaths;
}

void AnythingConfigWatcher::handleConfigChanged(const QString &key)
{
    if (key == kDeepinAnythingDconfPathKey) {
        defaultAnythingIndexPathsRealtime();
        fmInfo() << "Anything indexing paths changed, index rebuild needed";
        emit rebuildRequired(QStringLiteral("anythingIndexPathsChanged"));
    } else if (key == kDeepinAnythingDconfBlacklistPathKey) {
        defaultBlacklistPathsRealtime();
        fmInfo() << "Anything blacklist paths changed, index rebuild needed";
        emit rebuildRequired(QStringLiteral("anythingBlacklistPathsChanged"));
    }
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
    defaultBlacklistPathsRealtime();
}

// ─────────────────────────────────────────────
// ConfigRebuildWatcher
// ─────────────────────────────────────────────

ConfigRebuildWatcher::ConfigRebuildWatcher(const QList<WatchEntry> &entries, QObject *parent)
    : QObject(parent)
{
    for (const auto &entry : entries) {
        ConfigHolder holder;
        holder.appId = entry.appId;
        holder.configId = entry.configId;
        holder.watchedKey = entry.key;
        holder.config = DConfig::create(entry.appId, entry.configId, "", this);

        if (!holder.config) {
            qWarning() << "ConfigRebuildWatcher: Failed to create DConfig for" << entry.appId << entry.configId;
            continue;
        }
        if (!holder.config->isValid()) {
            qWarning() << "ConfigRebuildWatcher: DConfig is not valid for" << entry.appId << entry.configId;
            holder.config->deleteLater();
            holder.config = nullptr;
            continue;
        }

        // Capture by value for the lambda closure,
        // since DConfig::valueChanged only provides the key, not the config source.
        const auto configId = entry.configId;
        const auto watchedKey = entry.key;
        connect(holder.config, &DConfig::valueChanged, this,
                [this, configId, watchedKey](const QString &key) {
                    if (key == watchedKey) {
                        const QString reason = QStringLiteral("%1:%2Changed").arg(configId, key);
                        fmInfo() << "ConfigRebuildWatcher:" << reason;
                        emit rebuildRequired(reason);
                    }
                });

        m_configs.append(holder);
    }
}

ConfigRebuildWatcher::~ConfigRebuildWatcher() = default;

// ─────────────────────────────────────────────
// DlnfsConfigWatcher
// ─────────────────────────────────────────────

DlnfsConfigWatcher *DlnfsConfigWatcher::instance()
{
    static DlnfsConfigWatcher *inst = new DlnfsConfigWatcher;
    return inst;
}

DlnfsConfigWatcher::~DlnfsConfigWatcher() = default;

DlnfsConfigWatcher::DlnfsConfigWatcher(QObject *parent)
    : QObject(parent),
      m_watcher(new ConfigRebuildWatcher({ { QStringLiteral("org.deepin.dde.file-manager"),
                                             QStringLiteral("org.deepin.dde.file-manager"),
                                             QStringLiteral("dfm.mount.dlnfs") },
                                           { QStringLiteral("org.deepin.dlnfs"),
                                             QStringLiteral("org.deepin.dlnfs"),
                                             QStringLiteral("ulnfs") } },
                                         this))
{
    connect(m_watcher, &ConfigRebuildWatcher::rebuildRequired,
            this, &DlnfsConfigWatcher::rebuildRequired);
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

QStringList extractAncestorPaths(const QString &filePath)
{
    QStringList ancestorPaths;
    if (filePath.isEmpty())
        return ancestorPaths;

    QFileInfo fileInfo(filePath);
    QString currentPath = fileInfo.path();   // 初始为文件所在的目录路径

    // 循环向上获取所有父目录，直到根目录
    while (currentPath != "/" && !currentPath.isEmpty()) {
        ancestorPaths.append(currentPath);
        currentPath = QFileInfo(currentPath).path();   // 获取父目录
    }

    // 如果文件路径是根目录下的文件，确保包括根目录
    if (fileInfo.path() == "/" && fileInfo.exists() && fileInfo.isFile()) {
        // 文件直接在根目录下，没有祖先目录
        return ancestorPaths;
    }

    // 添加根目录如果当前路径是 "/" 但还没有添加（处理目录路径的情况）
    if (currentPath == "/" && !ancestorPaths.contains("/")) {
        // 这种情况应该不会发生，因为文件路径是文件不是目录，fileInfo.path() 只会在文件直接在根目录下时返回 "/"
        // 而上面的条件已经处理了这种情况
    }

    return ancestorPaths;
}

}   // namespace PathCalculator

SERVICETEXTINDEX_END_NAMESPACE
