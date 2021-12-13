/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dfmstandardpaths.h"

#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QMap>
#include <QUrl>

DFMBASE_USE_NAMESPACE

QString DFMStandardPaths::location(DFMStandardPaths::StandardLocation type)
{
    switch (type) {
    case kTrashPath:
        return QDir::homePath() + "/.local/share/Trash";
    case kTrashExpungedPath:
        return QDir::homePath() + "/.local/share/Trash/expunged";
    case kTrashFilesPath:
        return QDir::homePath() + "/.local/share/Trash/files";
    case kTrashInfosPath:
        return QDir::homePath() + "/.local/share/Trash/info";
#ifdef APPSHAREDIR
    case kTranslationPath: {
        QString path = APPSHAREDIR "/translations";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/translations";
        }
        return path;
    }
    case kTemplatesPath: {
        QString path = APPSHAREDIR "/templates";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/templates";
        }
        return path;
    }
    case kMimeTypePath: {
        QString path = APPSHAREDIR "/mimetypes";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/mimetypes";
        }
        return path;
    }
    case kExtensionsPath: {
        QString path = APPSHAREDIR "/extensions";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/extensions";
        }
        return path;
    }
    case kExtensionsAppEntryPath: {
        QString path = APPSHAREDIR "/extensions/appEntry";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/extensions/appEntry";
        }
        return path;
    }
#endif
#ifdef PLUGINDIR
    case PluginsPath: {
        QString path = PLUGINDIR;
        if (!QDir(path).exists()) {
            path = QString::fromLocal8Bit(PLUGINDIR).split(':').last();
        }
        return path;
    }
#endif
#ifdef QMAKE_TARGET
    case ApplicationConfigPath:
        return getConfigPath();
#endif
    case kThumbnailPath: {
        const QString &cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
        return cachePath + "/thumbnails";
    }
    case kThumbnailFailPath:
        return location(kThumbnailPath) + "/fail";
    case kThumbnailLargePath:
        return location(kThumbnailPath) + "/large";
    case kThumbnailNormalPath:
        return location(kThumbnailPath) + "/normal";
    case kThumbnailSmallPath:
        return location(kThumbnailPath) + "/small";
#ifdef APPSHAREDIR
    case kApplicationSharePath:
        return APPSHAREDIR;
#endif
    case kRecentPath:
        return "recent:///";
    case kHomePath:
        return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    case kDesktopPath:
        return QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    case kVideosPath:
        return QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
    case kMusicPath:
        return QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    case kPicturesPath:
        return QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    case kDocumentsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
    case kDownloadsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first();
    case kCachePath:
        return getCachePath();
    case kDiskPath:
        return QDir::rootPath();
#ifdef NETWORK_ROOT
    case NetworkRootPath:
        return NETWORK_ROOT;
#endif
#ifdef USERSHARE_ROOT
    case UserShareRootPath:
        return USERSHARE_ROOT;
#endif
#ifdef COMPUTER_ROOT
    case ComputerRootPath:
        return COMPUTER_ROOT;
#endif
    case kRoot:
        return "/";
    case kVault:
        return "dfmvault:///";   // 根据需求确定使用哪种类型
    default:
        return QStringLiteral("bug://dde-file-manager-lib/interface/dfmstandardpaths.cpp#") + QT_STRINGIFY(type);
    }
}

QString DFMStandardPaths::fromStandardUrl(const QUrl &standardUrl)
{
    if (standardUrl.scheme() != "standard")
        return QString();

    static const QMap<QString, QString> pathConverts {
        { "home", location(kHomePath) },
        { "desktop", location(kDesktopPath) },
        { "videos", location(kVideosPath) },
        { "music", location(kMusicPath) },
        { "pictures", location(kPicturesPath) },
        { "documents", location(kDocumentsPath) },
        { "downloads", location(kDownloadsPath) }
    };

    const QString &path = pathConverts.value(standardUrl.host());

    if (path.isEmpty())
        return path;

    const QString &urlPath = standardUrl.path();

    if (urlPath.isEmpty() || urlPath == "/")
        return path;

    return path + standardUrl.path();
}

QUrl DFMStandardPaths::toStandardUrl(const QString &localPath)
{
    static const QList<QPair<QString, QString>> pathConverts {
        { location(kDesktopPath), "desktop" },
        { location(kVideosPath), "videos" },
        { location(kMusicPath), "music" },
        { location(kPicturesPath), "pictures" },
        { location(kDocumentsPath), "documents" },
        { location(kDownloadsPath), "downloads" },
        { location(kHomePath), "home" }
    };

    auto it = std::find_if(pathConverts.begin(), pathConverts.end(), [localPath](const QPair<QString, QString> &pathConvert) {
        const QString &pathFirst = pathConvert.first;
        const QString &path = localPath.mid(pathFirst.size());
        return localPath.startsWith(pathFirst) && (path.isEmpty() || path.startsWith("/"));
    });

    if (it != pathConverts.end()) {
        const QString &valueFirst = (*it).first;
        const QString &valueSecond = (*it).second;
        const QString &path = localPath.mid(valueFirst.size());

        QUrl url;
        url.setScheme("standard");
        url.setHost(valueSecond);

        if (!path.isEmpty() && path != "/")
            url.setPath(path);

        return url;
    }

    return QUrl();
}

#ifdef QMAKE_TARGET
QString DFMStandardPaths::getConfigPath()
{
    QString projectName = QMAKE_TARGET;

    const QString &configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    QDir::home().mkpath(configPath);

    const QString &projectPath = QString("%1/%2/%3/").arg(configPath, qApp->organizationName(), projectName);
    QDir::home().mkpath(projectPath);

    return projectPath;
}
#endif

QString DFMStandardPaths::getCachePath()
{
    QString projectName = qApp->applicationName();

    const QString &cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    QDir::home().mkpath(cachePath);

    const QString &projectPath = QString("%1/%2/%3/").arg(cachePath, qApp->organizationName(), projectName);
    QDir::home().mkpath(projectPath);

    return projectPath;
}

DFMStandardPaths::DFMStandardPaths()
{
}
