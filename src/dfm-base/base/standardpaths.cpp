/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "base/standardpaths.h"

#include <QUrl>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QMap>

DFMBASE_BEGIN_NAMESPACE
/*!
 * \class StandardPaths 基础路径提供类
 *
 * \brief 内部使用不同的enum来获取基本公用的文件路径
 *
 */

/*!
 * \brief StandardPaths::location 根据不同的StandardLocation类型获取相应的文件路径
 *
 * \param type StandardLocation类型
 *
 * \return QString tandardLocation类型对应的文件路径
 */
QString StandardPaths::location(StandardPaths::StandardLocation type)
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
    case kThumbnailPath:
        return QDir::homePath() + "/.cache/thumbnails";
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
    default:
        return QStringLiteral("bug://dde-file-manager-lib/interface/dfmstandardpaths.cpp#") + QT_STRINGIFY(type);
    }
}
/*!
 * \brief StandardPaths::iconName 获取不同StandardLocation类型的ICON
 *
 * \param type StandardLocation类型
 *
 * \return QString StandardLocation类型的对应icon的地址
 */
QString StandardPaths::iconName(StandardPaths::StandardLocation type)
{
    switch (type) {
    case kHomePath:
        return "user-home";
    case kDesktopPath:
        return "user-desktop";
    case kVideosPath:
        return "folder-videos";
    case kMusicPath:
        return "folder-music";
    case kPicturesPath:
        return "folder-pictures";
    case kDocumentsPath:
        return "folder-documents";
    case kDownloadsPath:
        return "folder-downloads";
    case kTrashPath:
        return "user-trash";
    case kRecentPath:
        return "document-open-recent";
    default:
        return "";
    }
}
/*!
 * \brief StandardPaths::displayName 获取不同StandardLocation类型的显示名称
 *
 * \param type StandardLocation类型
 *
 * \return QString StandardLocation类型的对应显示名称
 */
QString StandardPaths::displayName(StandardPaths::StandardLocation type)
{
    switch (type) {
    case kHomePath:
        return QObject::tr("Home");
    case kDesktopPath:
        return QObject::tr("Desktop");
    case kVideosPath:
        return QObject::tr("Videos");
    case kMusicPath:
        return QObject::tr("Music");
    case kPicturesPath:
        return QObject::tr("Pictures");
    case kDocumentsPath:
        return QObject::tr("Documents");
    case kDownloadsPath:
        return QObject::tr("Downloads");
    case kTrashPath:
        return QObject::tr("Trash");
    case kRecentPath:
        return QObject::tr("Recent");
    default:
        return QObject::tr("");
    }
}
/*!
 * \brief StandardPaths::fromStandardUrl 获取快捷目录的文件路径（主目录、桌面、视频、音乐、图片、文档、下载）
 *
 * \param url 快捷目录的Url
 *
 * \return 快捷目录的路径
 */
QString StandardPaths::fromStandardUrl(const QUrl &url)
{
    if (url.scheme() != "standard")
        return QString();

    static QMap<QString, QString> path_convert {
        { "home", location(kHomePath) },
        { "desktop", location(kDesktopPath) },
        { "videos", location(kVideosPath) },
        { "music", location(kMusicPath) },
        { "pictures", location(kPicturesPath) },
        { "documents", location(kDocumentsPath) },
        { "downloads", location(kDownloadsPath) }
    };

    const QString &path = path_convert.value(url.host());

    if (path.isEmpty())
        return path;

    const QString &url_path = url.path();

    if (url_path.isEmpty() || url_path == "/")
        return path;

    return path + url.path();
}
/*!
 * \brief StandardPaths::toStandardUrl 获取基本路径的QUrl
 *
 * \param localPath 基本路径
 *
 * \return QUrl 基本路径的url
 */
QUrl StandardPaths::toStandardUrl(const QString &localPath)
{
    static QList<QPair<QString, QString>> path_convert {
        { location(kDesktopPath), "desktop" },
        { location(kVideosPath), "videos" },
        { location(kMusicPath), "music" },
        { location(kPicturesPath), "pictures" },
        { location(kDocumentsPath), "documents" },
        { location(kDownloadsPath), "downloads" },
        { location(kHomePath), "home" }
    };

    for (auto begin : path_convert) {
        if (localPath.startsWith(begin.first)) {
            const QString &path = localPath.mid(begin.first.size());

            if (!path.isEmpty() && !path.startsWith("/"))
                continue;

            QUrl url;

            url.setScheme("standard");
            url.setHost(begin.second);

            if (!path.isEmpty() && path != "/")
                url.setPath(path);

            return url;
        }
    }

    return QUrl();
}
/*!
 * \brief StandardPaths::getCachePath 获取工程的cache目录
 *
 * \return QString 工程的cache目录路径
 */
QString StandardPaths::getCachePath()
{
    QString projectName = qApp->applicationName();
    QDir::home().mkpath(".cache");
    QDir::home().mkpath(QString("%1/deepin/%2/").arg(".cache", projectName));
    QString defaultPath = QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".cache", projectName);
    return defaultPath;
}

StandardPaths::StandardPaths()
{
}
DFMBASE_END_NAMESPACE
