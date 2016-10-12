#include "standardpath.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include "./controllers/pathmanager.h"

StandardPath::StandardPath()
{

}

StandardPath::~StandardPath()
{

}

QString StandardPath::getAppConfigPath()
{
    return APPSHAREDIR;
}

QString StandardPath::getHomePath()
{
    return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
}

QString StandardPath::getDesktopPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
}

QString StandardPath::getVideosPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0);
}

QString StandardPath::getMusicPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0);
}

QString StandardPath::getPicturesPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
}

QString StandardPath::getDocumentsPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
}

QString StandardPath::getDownloadsPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).at(0);
}

QString StandardPath::getCachePath()
{
    QString projectName = qApp->applicationName();
    QDir::home().mkpath(".cache");
    QDir::home().mkpath(QString("%1/%2/").arg(".cache", projectName));
    QString defaultPath = QString("%1/%2/%3").arg(QDir::homePath(), ".cache", projectName);
    return defaultPath;
}

QString StandardPath::getConfigPath()
{
    QString projectName = qApp->applicationName();
    QDir::home().mkpath(".config");
    QDir::home().mkpath(QString("%1/%2/").arg(".config", projectName));
    QString defaultPath = QString("%1/%2/%3").arg(QDir::homePath(), ".config", projectName);
    return defaultPath;
}

QString StandardPath::getTrashPath()
{
    QDir::home().mkpath(".local/");
    QDir::home().mkpath(".local/share/");
    QDir::home().mkpath( ".local/share/Trash/" );
    QDir::home().mkpath( ".local/share/Trash/files/" );
    QDir::home().mkpath( ".local/share/Trash/info/" );
    return QDir::homePath() + "/.local/share/Trash/";
}

QString StandardPath::getTrashFilesPath()
{
    return QDir::homePath() + "/.local/share/Trash/files/";
}

QString StandardPath::getDiskPath()
{
    return QDir::rootPath();
}

QString StandardPath::getNetworkRootPath()
{
    return NETWORK_ROOT;
}

QString StandardPath::getUserShareRootPath()
{
    return USERSHARE_ROOT;
}

