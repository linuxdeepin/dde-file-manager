// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "directorymodel.h"
#include "networkguard.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSettings>
#include <QLocale>
#include <QLoggingCategory>
#include <QUrl>
#include <QtConcurrent>
#include <utility>

#include <sys/stat.h>

#include <DThumbnailProvider>

Q_LOGGING_CATEGORY(fileManagerDirModelLog, "org.deepin.dde.shell.filemanager.directorymodel")

namespace dock {

namespace {
// Qt 6.8 exposes no file identity API, so a rename is detected by comparing
// the POSIX device/inode pair of the watched directory with its siblings.
bool statDirectoryIdentity(const QString &path, quint64 *device, quint64 *inode)
{
    if (path.isEmpty()) return false;
    struct stat info {};
    if (::stat(QFile::encodeName(path).constData(), &info) != 0) return false;
    if (!S_ISDIR(info.st_mode)) return false;
    *device = static_cast<quint64>(info.st_dev);
    *inode = static_cast<quint64>(info.st_ino);
    return true;
}
}

DirectoryModel::DirectoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.setInterval(100);
    connect(&m_reloadTimer, &QTimer::timeout, this, &DirectoryModel::refresh);
    // Wait a little before declaring a pinned folder deleted: a rename handled
    // by the same watcher tick has to win, and a delete/re-create cycle must
    // not flap the dock icon.
    m_lossTimer.setSingleShot(true);
    m_lossTimer.setInterval(500);
    connect(&m_lossTimer, &QTimer::timeout, this, [this] {
        // An unreachable mount would hang on stat(); the tree may simply be
        // temporarily offline, so keep the pin until it responds again.
        if (m_path.isEmpty() || NetworkGuard::instance().isBusy(m_path))
            return;
        if (!QFileInfo(m_path).isDir())
            Q_EMIT pathLost();
    });
    connect(&m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, [this] { m_reloadTimer.start(); });
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [this] { m_reloadTimer.start(); });

    if (auto *provider = Dtk::Gui::DThumbnailProvider::instance()) {
        connect(provider, &Dtk::Gui::DThumbnailProvider::thumbnailChanged,
                this, &DirectoryModel::onThumbnailGenerated);
        connect(provider, &Dtk::Gui::DThumbnailProvider::createThumbnailFinished,
                this, &DirectoryModel::onThumbnailGenerated);
    }
}

int DirectoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.size();
}

QVariant DirectoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const auto &entry = m_entries.at(index.row());
    switch (role) {
    case NameRole:   return entry.name;
    case PathRole:   return entry.path;
    case IconNameRole: return entry.iconName;
    case IsDirRole:  return entry.isDir;
    case FileTypeRole: return entry.fileType;
    case ThumbnailUrlRole: return entry.thumbnailUrl;
    }
    return {};
}

QHash<int, QByteArray> DirectoryModel::roleNames() const
{
    return {
        {NameRole,    "fileName"},
        {PathRole,    "filePath"},
        {IconNameRole, "iconName"},
        {IsDirRole,   "isDir"},
        {FileTypeRole, "fileType"},
        {ThumbnailUrlRole, "thumbnailUrl"},
    };
}

QString DirectoryModel::path() const
{
    return m_path;
}

void DirectoryModel::setPath(const QString &path)
{
    const QFileInfo info(path);
    const auto normalized = info.absoluteFilePath();
    if (m_path == normalized)
        return;
    m_path = normalized;
    // An unreachable network mount would hang the stat below and every
    // subsequent directory access; present an empty folder instead until the
    // host responds again.
    if (NetworkGuard::instance().isBusy(normalized)) {
        setNetworkBusy(true);
        beginResetModel();
        m_entries.clear();
        m_folderCount = 0;
        endResetModel();
        Q_EMIT countChanged();
        Q_EMIT pathChanged();
        return;
    }
    setNetworkBusy(false);
    // Remember the identity of the path we are about to show, so a later
    // rename in the same parent directory can be recognised.
    if (!statDirectoryIdentity(normalized, &m_pathDevice, &m_pathInode)) {
        m_pathDevice = 0;
        m_pathInode = 0;
    }
    beginResetModel();
    m_entries.clear();
    m_folderCount = 0;
    endResetModel();
    Q_EMIT countChanged();
    Q_EMIT pathChanged();
    loadDirectory();
}

int DirectoryModel::folderCount() const
{
    return m_folderCount;
}

int DirectoryModel::totalCount() const
{
    return m_entries.size();
}

void DirectoryModel::refresh()
{
    loadDirectory();
}

bool DirectoryModel::canGoBack() const
{
    return m_historyIndex > 0;
}

bool DirectoryModel::canGoForward() const
{
    return m_historyIndex < m_history.size() - 1;
}

void DirectoryModel::navigateTo(const QString &path)
{
    // Probe before touching the file system: isDir/isReadable below would
    // block on an unreachable network mount.
    if (path.isEmpty() || NetworkGuard::instance().isBusy(path))
        return;
    const QFileInfo info(path);
    if (!info.isDir() || !info.isReadable())
        return;

    // Truncate forward history
    m_history = m_history.mid(0, m_historyIndex + 1);
    m_history.append(path);
    m_historyIndex = m_history.size() - 1;

    static constexpr int kMaxHistory = 100;
    if (m_history.size() > kMaxHistory) {
        m_history = m_history.mid(m_history.size() - kMaxHistory);
        m_historyIndex = m_history.size() - 1;
    }

    setPath(path);
    Q_EMIT navigationChanged();
}

void DirectoryModel::goBack()
{
    if (!canGoBack())
        return;
    m_historyIndex--;
    setPath(m_history.at(m_historyIndex));
    Q_EMIT navigationChanged();
}

void DirectoryModel::goForward()
{
    if (!canGoForward())
        return;
    m_historyIndex++;
    setPath(m_history.at(m_historyIndex));
    Q_EMIT navigationChanged();
}

QVariantMap DirectoryModel::get(int index) const
{
    if (index < 0 || index >= m_entries.size())
        return {};
    const auto &entry = m_entries.at(index);
    return {
        {QStringLiteral("fileName"),  entry.name},
        {QStringLiteral("filePath"),  entry.path},
        {QStringLiteral("iconName"),  entry.iconName},
        {QStringLiteral("isDir"),     entry.isDir},
        {QStringLiteral("fileType"),  entry.fileType},
        {QStringLiteral("thumbnailUrl"), entry.thumbnailUrl},
    };
}

void DirectoryModel::loadDirectory()
{
    ++m_loadGeneration;
    const int generation = m_loadGeneration;
    // The existence probes below would hang on an unreachable network mount;
    // report an empty folder and retry when something triggers a refresh.
    if (NetworkGuard::instance().isBusy(m_path)) {
        setNetworkBusy(true);
        LoadResult empty;
        empty.generation = generation;
        applyResult(empty);
        return;
    }
    setNetworkBusy(false);
    updateWatches();

    if (!QDir(m_path).exists()) {
        // The path may have been renamed in place. Adopt the new name instead
        // of reporting the pin as an empty folder.
        const QString renamed = findRenamedPath();
        if (!renamed.isEmpty()) {
            m_lossTimer.stop();
            const QString oldPath = m_path;
            setPath(renamed);
            Q_EMIT pathMoved(oldPath, renamed);
            return;
        }
        // Not a rename. When the parent directory is still there the folder
        // itself was deleted, so report it once the grace period expires and
        // the pin can be dropped. A missing parent keeps the pin: the tree may
        // simply be temporarily unmounted or unavailable.
        if (parentDirectoryAlive())
            m_lossTimer.start();
        else
            m_lossTimer.stop();
        LoadResult empty;
        empty.generation = generation;
        applyResult(empty);
        return;
    }
    m_lossTimer.stop();

    const QString path = m_path;
    auto watcher = new QFutureWatcher<LoadResult>(this);
    connect(watcher, &QFutureWatcher<LoadResult>::finished, this, [this, watcher] {
        applyResult(watcher->result());
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run([path, generation]() {
        return collectEntries(path, generation);
    }));
}

DirectoryModel::LoadResult DirectoryModel::collectEntries(const QString &path, int generation)
{
    LoadResult result;
    result.generation = generation;

    QDir dir(path);
    if (!dir.exists())
        return result;

    QMimeDatabase mimeDb;

    const auto entries = dir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    for (const auto &info : entries) {
        Entry entry;
        entry.name = info.fileName();
        entry.path = info.absoluteFilePath();
        entry.isDir = info.isDir();

        if (entry.isDir) {
            entry.iconName = QStringLiteral("folder");
            entry.fileType = Folder;
            result.folderCount++;
        } else {
            auto mime = mimeDb.mimeTypeForFile(info);
            entry.iconName = mime.iconName();
            if (entry.iconName.isEmpty())
                entry.iconName = mime.genericIconName();
            if (entry.iconName.isEmpty())
                entry.iconName = QStringLiteral("text-x-generic");

            // Classify file type by MIME category
            QString mimeName = mime.name();
            if (mimeName.startsWith(QLatin1String("image/")))
                entry.fileType = ImageFile;
            else if (mimeName.startsWith(QLatin1String("video/")))
                entry.fileType = VideoFile;
            else if (mimeName.startsWith(QLatin1String("audio/")))
                entry.fileType = AudioFile;
            else if (info.suffix() == QLatin1String("desktop"))
                entry.fileType = DesktopFile;

            // Detect script files by MIME type or shebang
            if (entry.fileType == GenericFile) {
                static const QStringList scriptMimes = {
                    QStringLiteral("application/x-shellscript"),
                    QStringLiteral("application/x-python"),
                    QStringLiteral("application/x-perl"),
                    QStringLiteral("application/x-ruby"),
                    QStringLiteral("text/x-script.python"),
                    QStringLiteral("text/x-script.sh"),
                    QStringLiteral("text/x-script.perl"),
                };
                if (scriptMimes.contains(mimeName))
                    entry.fileType = ScriptFile;
            }

            // Resolve the application icon and localized name from .desktop files
            if (entry.fileType == DesktopFile) {
                QSettings desktopFile(entry.path, QSettings::IniFormat);
                desktopFile.beginGroup(QStringLiteral("Desktop Entry"));

                QString desktopIcon = desktopFile.value(QStringLiteral("Icon")).toString();
                if (!desktopIcon.isEmpty())
                    entry.iconName = desktopIcon;

                QString langSuffix = QLocale().name();   // e.g. "zh_CN"
                QString localizedName = desktopFile.value(QStringLiteral("Name[%1]").arg(langSuffix)).toString();
                if (localizedName.isEmpty())
                    localizedName = desktopFile.value(QStringLiteral("Name")).toString();
                if (!localizedName.isEmpty())
                    entry.name = localizedName;

                desktopFile.endGroup();
            }
        }

        result.entries.append(entry);
    }

    return result;
}

void DirectoryModel::applyResult(LoadResult result)
{
    if (result.generation != m_loadGeneration)
        return;

    // DThumbnailProvider owns a shared MIME cache with lazy initialization.
    // Access it on the GUI thread: independent directory scans must not race
    // to initialize that cache from QtConcurrent workers.
    QVector<QFileInfo> pendingThumbnails;
    // hasThumbnail may stat the source file; skip the whole thumbnail pass
    // while the mount is unreachable so the GUI thread cannot hang here.
    const bool busy = NetworkGuard::instance().isBusy(m_path);
    for (auto &entry : result.entries) {
        if (entry.isDir || entry.fileType == DesktopFile || busy) continue;
        const QFileInfo info(entry.path);
        bool needsGeneration = false;
        entry.thumbnailUrl = cachedThumbnailUrl(info, &needsGeneration);
        if (needsGeneration) pendingThumbnails.append(info);
    }

    beginResetModel();
    m_entries = std::move(result.entries);
    m_folderCount = result.folderCount;
    endResetModel();
    updateWatches();
    Q_EMIT countChanged();

    enqueueThumbnails(pendingThumbnails);
}

QString DirectoryModel::findRenamedPath() const
{
    // Only a rename inside the same parent keeps the entry discoverable by a
    // sibling scan; a move to another directory is not followed.
    if (m_pathInode == 0 || m_path.isEmpty())
        return {};

    const QFileInfo root(m_path);
    const QDir parent(root.absolutePath());
    if (!parent.exists())
        return {};

    const auto siblings = parent.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
    for (const auto &sibling : siblings) {
        if (sibling.absoluteFilePath() == root.absoluteFilePath()) continue;
        quint64 device = 0, inode = 0;
        if (!statDirectoryIdentity(sibling.absoluteFilePath(), &device, &inode)) continue;
        if (inode == m_pathInode && device == m_pathDevice)
            return sibling.absoluteFilePath();
    }
    return {};
}

bool DirectoryModel::parentDirectoryAlive() const
{
    if (m_path.isEmpty())
        return false;
    const QFileInfo root(m_path);
    const QString parent = root.absolutePath();
    // A root-level path has no meaningful parent to test.
    return parent != root.absoluteFilePath() && QFileInfo(parent).isDir();
}

void DirectoryModel::updateWatches()
{
    // Refreshing the watch list stats every entry; an unreachable mount would
    // hang here, and inotify on a dead mount is useless anyway.
    if (NetworkGuard::instance().isBusy(m_path))
        return;

    // Observe the nearest existing ancestor as well, so a missing pinned path
    // can recover even if several parent directories have been removed.
    QStringList wanted;
    if (!m_path.isEmpty()) {
        const QFileInfo root(m_path);
        QString ancestor = root.absolutePath();
        while (!QFileInfo(ancestor).isDir()) {
            const QString parent = QFileInfo(ancestor).absolutePath();
            if (parent == ancestor) break;
            ancestor = parent;
        }
        wanted << ancestor;
        if (root.isDir()) wanted << root.absoluteFilePath();
        for (const auto &entry : std::as_const(m_entries))
            if (QFileInfo::exists(entry.path)) wanted << entry.path;
    }
    wanted.removeDuplicates();
    const QStringList current = m_fileWatcher.files() + m_fileWatcher.directories();
    for (const auto &path : current)
        if (!wanted.contains(path)) m_fileWatcher.removePath(path);
    for (const auto &path : wanted)
        if (!current.contains(path)) m_fileWatcher.addPath(path);
}

void DirectoryModel::setNetworkBusy(bool busy)
{
    if (m_networkBusy == busy)
        return;
    m_networkBusy = busy;
    qCInfo(fileManagerDirModelLog) << "Network mount" << m_path << (busy ? "is unreachable" : "is back");
    Q_EMIT networkBusyChanged();
}

void DirectoryModel::onThumbnailGenerated(const QString &sourceFilePath, const QString &thumbnailPath)
{
    if (sourceFilePath.isEmpty() || thumbnailPath.isEmpty())
        return;

    const QString cleanSourcePath = QDir::cleanPath(sourceFilePath);
    for (int i = 0; i < m_entries.size(); ++i) {
        if (QDir::cleanPath(m_entries[i].path) == cleanSourcePath) {
            m_entries[i].thumbnailUrl = QUrl::fromLocalFile(thumbnailPath).toString();
            Q_EMIT thumbnailChanged(i);
            const auto idx = index(i, 0);
            Q_EMIT dataChanged(idx, idx, {ThumbnailUrlRole});
            return;
        }
    }
}

QString DirectoryModel::cachedThumbnailUrl(const QFileInfo &fileInfo, bool *needsGeneration)
{
    if (needsGeneration)
        *needsGeneration = false;

    if (fileInfo.isDir())
        return {};

    if (fileInfo.suffix() == QLatin1String("desktop"))
        return {};

    auto *provider = Dtk::Gui::DThumbnailProvider::instance();
    if (!provider || !provider->hasThumbnail(fileInfo))
        return {};

    const QString thumbnailPath = provider->thumbnailFilePath(fileInfo, Dtk::Gui::DThumbnailProvider::Small);
    if (!thumbnailPath.isEmpty() && QFileInfo::exists(thumbnailPath))
        return QUrl::fromLocalFile(thumbnailPath).toString();

    // No cached thumbnail yet: ask the GUI thread to request generation.
    if (needsGeneration)
        *needsGeneration = true;
    return {};
}

void DirectoryModel::enqueueThumbnails(const QVector<QFileInfo> &files)
{
    auto *provider = Dtk::Gui::DThumbnailProvider::instance();
    if (!provider)
        return;

    for (const auto &info : files)
        provider->appendToProduceQueue(info, Dtk::Gui::DThumbnailProvider::Small);
}

}
