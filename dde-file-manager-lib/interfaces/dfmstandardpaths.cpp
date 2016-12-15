#include "dfmstandardpaths.h"

#include <QDir>
#include <QApplication>
#include <QStandardPaths>
#include "durl.h"

QString DFMStandardPaths::standardLocation(DFMStandardPaths::StandardLocation type)
{
    switch (type) {
    case TrashPath:
        return QDir::homePath() + "/.local/share/Trash";
    case TrashFilesPath:
        return QDir::homePath() + "/.local/share/Trash/files";
    case TrashInfosPath:
        return QDir::homePath() + "/.local/share/Trash/info";
    case TranslationPath:
        return APPSHAREDIR"/translations";
    case ApplicationConfigPath:
        return getConfigPath();
    case ThumbnailPath:
        return QDir::homePath() + "/.cache/thumbnails";
    case ThumbnailFailPath:
        return standardLocation(ThumbnailPath) + "/fail";
    case ThumbnailLargePath:
        return standardLocation(ThumbnailPath) + "/large";
    case ThumbnailNormalPath:
        return standardLocation(ThumbnailPath) + "/normal";
    case ThumbnailSmallPath:
        return standardLocation(ThumbnailPath) + "/small";
    case ApplicationSharePath:
        return APPSHAREDIR;
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
    case NetworkRootPath:
        return NETWORK_ROOT;
    case UserShareRootPath:
        return USERSHARE_ROOT;
    case ComputerRoorPath:
        return COMPUTER_ROOT;
    }

    return QString();
}

QString DFMStandardPaths::getConfigPath()
{
    QString projectName = QMAKE_TARGET;
    QDir::home().mkpath(".config");
    QDir::home().mkpath(QString("%1/%2/").arg(".config", projectName));
    QString defaultPath = QString("%1/%2/%3").arg(QDir::homePath(), ".config", projectName);
    return defaultPath;
}

QString DFMStandardPaths::getCachePath()
{
    QString projectName = qApp->applicationName();
    QDir::home().mkpath(".cache");
    QDir::home().mkpath(QString("%1/%2/").arg(".cache", projectName));
    QString defaultPath = QString("%1/%2/%3").arg(QDir::homePath(), ".cache", projectName);
    return defaultPath;
}

DFMStandardPaths::DFMStandardPaths()
{
}
