/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
    case HomePath:
        return "user-home";
    case DesktopPath:
        return "user-desktop";
    case VideosPath:
        return "folder-videos";
    case MusicPath:
        return "folder-music";
    case PicturesPath:
        return "folder-pictures";
    case DocumentsPath:
        return "folder-documents";
    case DownloadsPath:
        return "folder-downloads";
    case TrashPath:
        return "user-trash";
    case RecentPath:
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
    case HomePath:
        return QObject::tr("Home");
    case DesktopPath:
        return QObject::tr("Desktop");
    case VideosPath:
        return QObject::tr("Videos");
    case MusicPath:
        return QObject::tr("Music");
    case PicturesPath:
        return QObject::tr("Pictures");
    case DocumentsPath:
        return QObject::tr("Documents");
    case DownloadsPath:
        return QObject::tr("Downloads");
    case TrashPath:
        return QObject::tr("Trash");
    case RecentPath:
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
        {"home", location(HomePath)},
        {"desktop", location(DesktopPath)},
        {"videos", location(VideosPath)},
        {"music", location(MusicPath)},
        {"pictures", location(PicturesPath)},
        {"documents", location(DocumentsPath)},
        {"downloads", location(DownloadsPath)}
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
