// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/standardpaths.h"

#include <QUrl>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QMap>

namespace dfmbase {
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
    case kTrashLocalPath:
        return QDir::homePath() + "/.local/share/Trash";
    case kTrashLocalFilesPath:
        return QDir::homePath() + "/.local/share/Trash/files";
    case kTrashLocalInfoPath:
        return QDir::homePath() + "/.local/share/Trash/info";
    case kApplicationConfigPath:
        return QDir::homePath() + "/.config";
#ifdef APPSHAREDIR
        // NOTE(xust): the APPSHAREDIR is associated with CMAKE_INSTALL_PREFIX
        // if the dir not exists in DEBUG mode, change the defination of CMAKE variable in your IDE config.
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
    case kVault:
        return "dfmvault:///";   // 根据需求确定使用哪种类型
    default:
        return QStringLiteral("bug://dde-file-manager-lib/interface/dfmstandardpaths.cpp#") + QT_STRINGIFY(type);
    }
}

QString StandardPaths::location(const QString &dirName)
{
    static QMap<QString, QString> pathConvert {
        { "home", location(kHomePath) },
        { "desktop", location(kDesktopPath) },
        { "videos", location(kVideosPath) },
        { "music", location(kMusicPath) },
        { "pictures", location(kPicturesPath) },
        { "documents", location(kDocumentsPath) },
        { "downloads", location(kDownloadsPath) }
    };

    return pathConvert.value(dirName, "");
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
    case kTrashLocalPath:
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
    case kTrashLocalPath:
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
QString StandardPaths::fromStandardUrl(const QUrl &standardUrl)
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
/*!
 * \brief StandardPaths::toStandardUrl 获取基本路径的QUrl
 *
 * \param localPath 基本路径
 *
 * \return QUrl 基本路径的url
 */
QUrl StandardPaths::toStandardUrl(const QString &localPath)
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
/*!
 * \brief StandardPaths::getCachePath 获取工程的cache目录
 *
 * \return QString 工程的cache目录路径
 */
QString StandardPaths::getCachePath()
{
    QString projectName = qApp->applicationName();

    const QString &cachePath = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first();
    QDir::home().mkpath(cachePath);

    const QString &projectPath = QString("%1/%2/%3/").arg(cachePath, qApp->organizationName(), projectName);
    QDir::home().mkpath(projectPath);

    return projectPath;
}

/*!
 * \brief StandardPaths::getDisplayName
 * \param dirName the directory name such as "videos/music/desktop"
 * \return the display name of directory
 */
QString StandardPaths::displayName(const QString &dirName)
{
    static QMap<QString, QString> datas {
        std::pair<QString, QString>("desktop", QObject::tr("Desktop")),
        std::pair<QString, QString>("videos", QObject::tr("Videos")),
        std::pair<QString, QString>("music", QObject::tr("Music")),
        std::pair<QString, QString>("pictures", QObject::tr("Pictures")),
        std::pair<QString, QString>("documents", QObject::tr("Documents")),
        std::pair<QString, QString>("downloads", QObject::tr("Downloads")),
    };
    return datas.value(dirName, QObject::tr("Unknown"));
}

QString StandardPaths::iconName(const QString &dirName)
{
    static QMap<QString, QString> datas {
        std::pair<QString, QString>("desktop", "user-desktop"),
        std::pair<QString, QString>("videos", "folder-videos"),
        std::pair<QString, QString>("music", "folder-music"),
        std::pair<QString, QString>("pictures", "folder-pictures"),
        std::pair<QString, QString>("documents", "folder-documents"),
        std::pair<QString, QString>("downloads", "folder-downloads"),
    };
    return datas.value(dirName, "folder");
}

StandardPaths::StandardPaths()
{
}
}
