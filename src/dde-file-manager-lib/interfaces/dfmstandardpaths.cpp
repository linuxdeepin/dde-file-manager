/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "durl.h"

#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QMap>

QString DFMStandardPaths::location(DFMStandardPaths::StandardLocation type)
{
    switch (type) {
    case TrashPath:
        return QDir::homePath() + "/.local/share/Trash";
    case TrashExpungedPath:
        return QDir::homePath() + "/.local/share/Trash/expunged";
    case TrashFilesPath:
        return QDir::homePath() + "/.local/share/Trash/files";
    case TrashInfosPath:
        return QDir::homePath() + "/.local/share/Trash/info";
#ifdef APPSHAREDIR
    case TranslationPath: {
        QString path = APPSHAREDIR"/translations";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/translations";
        }
        return path;
    }
    case TemplatesPath: {
        QString path = APPSHAREDIR"/templates";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/templates";
        }
        return path;
    }
    case MimeTypePath: {
        QString path = APPSHAREDIR"/mimetypes";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/mimetypes";
        }
        return path;
    }
    case ExtensionsPath: {
        QString path = APPSHAREDIR"/extensions";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/extensions";
        }
        return path;
    }
    case ExtensionsAppEntryPath: {
        QString path = APPSHAREDIR"/extensions/appEntry";
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
    case ThumbnailPath:
        return QDir::homePath() + "/.cache/thumbnails";
    case ThumbnailFailPath:
        return location(ThumbnailPath) + "/fail";
    case ThumbnailLargePath:
        return location(ThumbnailPath) + "/large";
    case ThumbnailNormalPath:
        return location(ThumbnailPath) + "/normal";
    case ThumbnailSmallPath:
        return location(ThumbnailPath) + "/small";
#ifdef APPSHAREDIR
    case ApplicationSharePath:
        return APPSHAREDIR;
#endif
    case RecentPath:
        return "recent:///";
    case HomePath:
        return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    case DesktopPath:
        return QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    case VideosPath:
        return QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
    case MusicPath:
        return QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    case PicturesPath:
        return QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    case DocumentsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
    case DownloadsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first();
    case CachePath:
        return getCachePath();
    case DiskPath:
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
    case Root:
        return "/";
    case Vault:
        return DFMVAULT_ROOT; // 根据需求确定使用哪种类型
    default:
        return QStringLiteral("bug://dde-file-manager-lib/interface/dfmstandardpaths.cpp#") + QT_STRINGIFY(type);
    }
}

QString DFMStandardPaths::fromStandardUrl(const DUrl &standardUrl)
{
    if (standardUrl.scheme() != "standard")
        return QString();

    static QMap<QString, QString> path_convert {
        {"home", location(HomePath)},
        {"desktop", location(DesktopPath)},
        {"videos", location(VideosPath)},
        {"music", location(MusicPath)},
        {"pictures", location(PicturesPath)},
        {"documents", location(DocumentsPath)},
        {"downloads", location(DownloadsPath)}
    };

    const QString &path = path_convert.value(standardUrl.host());

    if (path.isEmpty())
        return path;

    const QString &url_path = standardUrl.path();

    if (url_path.isEmpty() || url_path == "/")
        return path;

    return path + standardUrl.path();
}

DUrl DFMStandardPaths::toStandardUrl(const QString &localPath)
{
    static QList<QPair<QString, QString>> path_convert {
        {location(DesktopPath), "desktop"},
        {location(VideosPath), "videos"},
        {location(MusicPath), "music"},
        {location(PicturesPath), "pictures"},
        {location(DocumentsPath), "documents"},
        {location(DownloadsPath), "downloads"},
        {location(HomePath), "home"}
    };

    for (auto begin : path_convert) {
        if (localPath.startsWith(begin.first)) {
            const QString &path = localPath.mid(begin.first.size());

            if (!path.isEmpty() && !path.startsWith("/"))
                continue;

            DUrl url;

            url.setScheme("standard");
            url.setHost(begin.second);

            if (!path.isEmpty() && path != "/")
                url.setPath(path);

            return url;
        }
    }

    return DUrl();
}

#ifdef QMAKE_TARGET
QString DFMStandardPaths::getConfigPath()
{
    QString projectName = QMAKE_TARGET;
    QDir::home().mkpath(".config");
    QDir::home().mkpath(QString("%1/deepin/%2/").arg(".config", projectName));
    QString defaultPath = QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".config", projectName);
    return defaultPath;
}
#endif

QString DFMStandardPaths::getCachePath()
{
    QString projectName = qApp->applicationName();
    QDir::home().mkpath(".cache");
    QDir::home().mkpath(QString("%1/deepin/%2/").arg(".cache", projectName));
    QString defaultPath = QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".cache", projectName);
    return defaultPath;
}

DFMStandardPaths::DFMStandardPaths()
{
}
