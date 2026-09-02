// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/utils/trashutils.h>

#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfmio_utils.h>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <unistd.h>
#include <atomic>

using namespace GlobalDConfDefines::ConfigPath;

DFMBASE_BEGIN_NAMESPACE

namespace TrashUtils {

static constexpr char kFileAllTrash[] { "dfm.trash.allfiletotrash" };

static std::atomic<int> kTrashEmptyState { static_cast<int>(TrashEmptyState::kUnknown) };

bool trashIsEmpty()
{
    const auto &dirs = localTrashDirs();
    for (const QString &dir : dirs) {
        QDir d(dir);
        if (!d.exists())
            continue;
        const QStringList &entries = d.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
        if (!entries.isEmpty())
            return false;
    }
    return true;
}

TrashEmptyState trashEmptyState()
{
    return static_cast<TrashEmptyState>(kTrashEmptyState.load());
}

void setTrashEmptyState(TrashEmptyState state)
{
    kTrashEmptyState.store(static_cast<int>(state));
}

QUrl trashRootUrl()
{
    QUrl url;
    url.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kTrash);
    url.setPath("/");
    url.setHost("");
    return url;
}

bool isTrashFile(const QUrl &url)
{
    if (url.scheme() == DFMBASE_NAMESPACE::Global::Scheme::kTrash)
        return true;
    if (url.path().startsWith(StandardPaths::location(StandardPaths::kTrashLocalFilesPath)))
        return true;

    const QString &rule = QString("/.Trash-%1/(files|info)/").arg(getuid());
    QRegularExpression reg(rule);
    QRegularExpressionMatch matcher = reg.match(url.toString());
    return matcher.hasMatch();
}

bool isTrashRootFile(const QUrl &url)
{
    if (UniversalUtils::urlEquals(url, trashRootUrl()))
        return true;

    if (UniversalUtils::urlEquals(url, QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kTrashLocalFilesPath))))
        return true;

    const QString &rule = QString("/.Trash-%1/files").arg(getuid());

    return url.toString().endsWith(rule);
}

bool fileCanTrash(const QUrl &url)
{
    auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
    if (!info) {
        qCWarning(logDFMBase) << "Failed to create file info for URL:" << url;
        return false;
    }

    // gio does not support root user to move ordinary user files to trash
    if (SysInfoUtils::isRootUser()) {
        int currentEffectiveUid = SysInfoUtils::getUserId();
        int ownerId = info->extendAttributes(FileInfo::FileExtendedInfoType::kOwnerId).toInt();
        if (ownerId != currentEffectiveUid) {
            qCWarning(logDFMBase) << "A root process is trying to trash a non-root file, predicting failure.";
            return false;
        }
    }

    // 检查文件是否位于原始用户的"领域"（主目录）内
    if (SysInfoUtils::isOpenAsAdmin()) {
        const QString &originalHomePath = SysInfoUtils::getOriginalUserHome();
        const QString &canonicalFilePath = info->pathOf(PathInfoType::kCanonicalPath);
        if (originalHomePath.isEmpty() || canonicalFilePath.isEmpty()) {
            qCWarning(logDFMBase) << "Invalid path detected: originalHomePath or canonicalFilePath is empty.";
            return false;
        }

        QString normalizedOriginalHome = QDir::cleanPath(originalHomePath);
        QString normalizedFilePath = QDir::cleanPath(canonicalFilePath);

        if (!normalizedOriginalHome.endsWith('/')) {
            normalizedOriginalHome += '/';
        }

        bool isInHomeDir = (normalizedFilePath.startsWith(normalizedOriginalHome) || normalizedFilePath == normalizedOriginalHome.chopped(1));

        if (isInHomeDir) {
            qCWarning(logDFMBase) << " A root process is trying to trash a file inside another user's home directory ("
                                  << url << "). Predicting GIO failure.";
            return false;
        }
    }

    bool alltotrash = DConfigManager::instance()->value(kDefaultCfgPath, kFileAllTrash).toBool();
    if (alltotrash)
        return info->canAttributes(CanableInfoType::kCanTrash);

    return ProtocolUtils::isLocalFile(url) && info->canAttributes(CanableInfoType::kCanTrash);
}

QString trashPathToNormal(const QString &trash)
{
    if (!trash.contains("\\"))
        return trash;
    QString normal = trash;
    normal = normal.replace("\\", "/");
    normal = normal.replace("//", "/");
    return normal;
}

QString normalPathToTrash(const QString &normal)
{
    QString trash = normal;
    trash = trash.replace("/", "\\");
    trash.push_front("/");
    return trash;
}

QStringList localTrashDirs()
{
    QStringList dirs;

    // Home trash (covers root filesystem and home directory)
    const QString &homeTrashFiles = StandardPaths::location(StandardPaths::kTrashLocalFilesPath);
    dirs.append(homeTrashFiles);

    // If allfiletotrash is not enabled, network files cannot be trashed,
    // so there is no need to traverse network mount points for trash dirs.
    const bool allToTrash = DConfigManager::instance()->value(kDefaultCfgPath, kFileAllTrash).toBool();

    // When allfiletotrash is enabled, still check CIFS reachability to avoid
    // blocking on stale network mounts.
    bool includeNetworkMounts = allToTrash;
    if (includeNetworkMounts && NetworkUtils::instance()->checkAllCIFSBusy())
        includeNetworkMounts = false;

    const uid_t uid = getuid();
    QFile mounts("/proc/mounts");
    if (mounts.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&mounts);
        QString line;
        while (stream.readLineInto(&line)) {
            const QStringList &parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() < 3)
                continue;

            const QString &device = parts[0];
            const QString &mountPoint = parts[1];

            // Skip remote/network filesystems unless allfiletotrash is enabled
            const bool isRemote = ProtocolUtils::isRemoteFile(QUrl::fromLocalFile(mountPoint))
                                  || device.startsWith("//");
            if (isRemote && !includeNetworkMounts)
                continue;

            // Skip root and home (already covered by home trash)
            if (mountPoint == "/" || mountPoint == QDir::homePath())
                continue;

            const QString &trashDir = QString("%1/.Trash-%2/files").arg(mountPoint).arg(uid);
            if (QDir(trashDir).exists() && !dirs.contains(trashDir))
                dirs.append(trashDir);
        }
    }

    return dirs;
}

TrashItemInfo resolveTrashUrl(const QUrl &url)
{
    TrashItemInfo info;

    if (url.scheme() != DFMBASE_NAMESPACE::Global::Scheme::kTrash)
        return info;

    const QString &fileName = url.fileName();
    if (fileName.isEmpty())
        return info;

    const auto &dirs = localTrashDirs();
    for (const QString &filesDir : dirs) {
        QString infoDir = filesDir;
        infoDir.replace("/files", "/info");
        const QString &infoFile = infoDir + "/" + fileName + ".trashinfo";

        QFile f(infoFile);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QTextStream ts(&f);
        QString line;
        bool inTrashInfoSection = false;
        while (ts.readLineInto(&line)) {
            line = line.trimmed();
            if (line.startsWith('[')) {
                inTrashInfoSection = (line == "[Trash Info]");
                continue;
            }
            if (!inTrashInfoSection)
                continue;
            if (line.startsWith("Path=", Qt::CaseInsensitive)) {
                QString path = line.mid(5);
                info.originalUrl = QUrl::fromLocalFile(path);
            } else if (line.startsWith("DeletionDate=", Qt::CaseInsensitive)) {
                info.deletionTime = QDateTime::fromString(line.mid(13), Qt::ISODate);
            }
        }

        const QString &localPath = filesDir + "/" + fileName;
        info.localFileUrl = QUrl::fromLocalFile(localPath);
        return info;
    }

    return info;
}

int countTrashItems()
{
    int count = 0;
    const auto &dirs = localTrashDirs();
    for (const QString &dir : dirs) {
        QDir d(dir);
        if (!d.exists())
            continue;
        QDirIterator it(dir, QDir::NoDotAndDotDot | QDir::AllEntries, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            if (it.fileInfo().isFile())
                ++count;
        }
    }
    return count;
}

qint64 calculateTrashSize()
{
    qint64 total = 0;
    const auto &dirs = localTrashDirs();
    for (const QString &dir : dirs) {
        QDir d(dir);
        if (!d.exists())
            continue;
        QDirIterator it(dir, QDir::NoDotAndDotDot | QDir::AllEntries, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            if (it.fileInfo().isFile())
                total += it.fileInfo().size();
        }
    }
    return total;
}

QUrl localFileToTrashUrl(const QString &localPath)
{
    QUrl url;
    url.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kTrash);

    const auto &dirs = localTrashDirs();
    for (const QString &dir : dirs) {
        if (localPath.startsWith(dir + "/") || localPath == dir) {
            QString relativePath = localPath.mid(dir.length() + 1);
            if (relativePath.isEmpty())
                url.setPath("/");
            else
                url.setPath("/" + relativePath);
            url.setHost("");
            return url;
        }
    }

    // Fallback: use just the file name
    url.setPath("/" + QFileInfo(localPath).fileName());
    url.setHost("");
    return url;
}

}   // namespace TrashUtils

DFMBASE_END_NAMESPACE
