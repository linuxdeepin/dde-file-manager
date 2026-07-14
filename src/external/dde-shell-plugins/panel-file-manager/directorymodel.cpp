// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "directorymodel.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QIcon>
#include <QPixmap>
#include <QBuffer>
#include <QSettings>
#include <QLocale>
#include <QLoggingCategory>
#include <QUrl>

#include <DThumbnailProvider>

Q_LOGGING_CATEGORY(fileManagerDirModelLog, "org.deepin.dde.shell.filemanager.directorymodel")

namespace dock {

DirectoryModel::DirectoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
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
    case IconUrlRole: return entry.iconUrl;
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
        {IconUrlRole, "iconUrl"},
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
    if (m_path == path)
        return;
    m_path = path;
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
    if (path.isEmpty())
        return;

    // Truncate forward history
    m_history = m_history.mid(0, m_historyIndex + 1);
    m_history.append(path);
    m_historyIndex = m_history.size() - 1;

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
        {QStringLiteral("iconUrl"),   entry.iconUrl},
        {QStringLiteral("iconName"),  entry.iconName},
        {QStringLiteral("isDir"),     entry.isDir},
        {QStringLiteral("fileType"),  entry.fileType},
        {QStringLiteral("thumbnailUrl"), entry.thumbnailUrl},
    };
}

void DirectoryModel::loadDirectory()
{
    beginResetModel();
    m_entries.clear();
    m_folderCount = 0;

    QDir dir(m_path);
    if (!dir.exists()) {
        endResetModel();
        Q_EMIT countChanged();
        return;
    }

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
            m_folderCount++;
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

        // Try to get real thumbnail for non-directory files
        entry.thumbnailUrl = thumbnailUrlForFile(info);

        entry.iconUrl = iconToDataUrl(entry.iconName, 64);
        m_entries.append(entry);
    }

    endResetModel();
    Q_EMIT countChanged();
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

QString DirectoryModel::thumbnailUrlForFile(const QFileInfo &fileInfo)
{
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

    provider->appendToProduceQueue(fileInfo, Dtk::Gui::DThumbnailProvider::Small);
    return {};
}

QString DirectoryModel::iconToDataUrl(const QString &iconName, int size)
{
    QIcon icon;
    if (iconName.startsWith(QLatin1Char('/')))
        icon = QIcon(iconName);
    if (icon.isNull())
        icon = QIcon::fromTheme(iconName);
    if (icon.isNull())
        icon = QIcon::fromTheme(QStringLiteral("text-x-generic"));
    if (icon.isNull())
        return QString();

    QPixmap pm = icon.pixmap(size, size);
    if (pm.isNull())
        return QString();

    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QIODevice::WriteOnly);
    pm.save(&buf, "PNG");
    return QStringLiteral("data:image/png;base64,") + ba.toBase64();
}

}
