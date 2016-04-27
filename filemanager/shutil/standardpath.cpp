#include "standardpath.h"
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QDebug>

StandardPath::StandardPath()
{

}

StandardPath::~StandardPath()
{

}

QString StandardPath::getHomePath()
{
    return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
}

QString StandardPath::getDesktopPath()
{
    return QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
}

QString StandardPath::getCachePath()
{
    QDir::home().mkpath(".cache");
    QDir::home().mkpath(QString("%1/%2/").arg(".cache", qApp->applicationName()));
    QString defaultPath = QString("%1/%2/%3").arg(QDir::homePath(), ".cache", qApp->applicationName());
    getTrashPath();
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

