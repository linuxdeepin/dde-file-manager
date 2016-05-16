#include "pathmanager.h"
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QDebug>
#include <QDir>
#include "../shutil/standardpath.h"

PathManager::PathManager(QObject *parent) : QObject(parent)
{
    m_fileSystemWatcher = new QFileSystemWatcher(this);
    initPaths();
    initConnect();
}

PathManager::~PathManager()
{

}

void PathManager::initPaths()
{
    loadSystemPaths();
    m_systemPathDisplayNames["Desktop"] = tr("Desktop");
    m_systemPathDisplayNames["Videos"] = tr("Videos");
    m_systemPathDisplayNames["Music"] = tr("Music");
    m_systemPathDisplayNames["Pictures"] = tr("Pictures");
    m_systemPathDisplayNames["Documents"] = tr("Documents");
    m_systemPathDisplayNames["Downloads"] = tr("Downloads");


    m_systemPathIconNames["Home"] = "folder-home";
    m_systemPathIconNames["Desktop"] = "folder-desktop";
    m_systemPathIconNames["Videos"] = "folder-videos";
    m_systemPathIconNames["Music"] = "folder-music";
    m_systemPathIconNames["Pictures"] = "folder-pictures";
    m_systemPathIconNames["Documents"] = "folder-documents";
    m_systemPathIconNames["Downloads"] = "folder-downloads";
}

void PathManager::initConnect()
{
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &PathManager::handleDirectoryChanged);
}

QString PathManager::getSystemPath(QString key)
{
    if (m_systemPaths.isEmpty()){
        initPaths();
    }
    QString path = m_systemPaths.value(key);
    if (!QDir(path).exists()){
        bool flag = QDir::home().mkpath(path);
        qDebug() << "mkpath" << path << flag;
    }
    return path;
}

QString PathManager::getSystemPathDisplayName(QString key)
{
    if (m_systemPathDisplayNames.contains(key))
        return m_systemPathDisplayNames.value(key);
    return QString();
}

QString PathManager::getSystemPathDisplayNameByPath(const QString &path)
{
    if (isSystemPath(path)){
        foreach (QString key, systemPaths().keys()) {
            if (systemPaths().value(key) == path){
                 return getSystemPathDisplayName(key);
            }
        }
    }
    return QString();
}


QString PathManager::getSystemPathIconName(QString key)
{
    if (m_systemPathIconNames.contains(key))
        return m_systemPathIconNames.value(key);
    return QString();
}

QString PathManager::getSystemPathIconNameByPath(const QString &path)
{
    if (isSystemPath(path)){
        foreach (QString key, systemPaths().keys()) {
            if (systemPaths().value(key) == path){
                 return getSystemPathIconName(key);
            }
        }
    }
    return QString();
}


QString PathManager::getSystemCachePath()
{
    return QString("%1/%2").arg(StandardPath::getCachePath(), "systempath.json");
}


void PathManager::loadSystemPaths()
{
    m_systemPaths["Home"] = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
    m_systemPaths["Desktop"] = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
    m_systemPaths["Videos"] = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0);
    m_systemPaths["Music"] = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0);
    m_systemPaths["Pictures"] = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
    m_systemPaths["Documents"] = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    m_systemPaths["Downloads"] = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).at(0);

    foreach (QString key, m_systemPaths.keys()) {
        QString path = m_systemPaths.value(key);
        mkPath(path);
        m_fileSystemWatcher->addPath(path);
    }
}

void PathManager::mkPath(const QString &path)
{
    if (!QDir(path).exists()){
        bool flag = QDir::home().mkpath(path);
        qDebug() << "mkpath" << path << flag;
    }
}

void PathManager::handleDirectoryChanged(const QString &path)
{
    qDebug() << path;
    mkPath(path);
    m_fileSystemWatcher->addPath(path);
}

QMap<QString, QString> PathManager::systemPathDisplayNames() const
{
    return m_systemPathDisplayNames;
}

bool PathManager::isSystemPath(const QString &path)
{
    return m_systemPaths.values().contains(path);
}



QMap<QString, QString> PathManager::systemPaths() const
{
    return m_systemPaths;
}


