#include "pathmanager.h"
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QDebug>
#include <QDir>
#include "shutil/standardpath.h"

PathManager::PathManager(QObject *parent) : QObject(parent)
{
    initPaths();
}

PathManager::~PathManager()
{

}

void PathManager::initPaths()
{
    loadSystemPaths();
    m_systemPathDisplayNamesMap["Home"] = tr("Home");
    m_systemPathDisplayNamesMap["Desktop"] = tr("Desktop");
    m_systemPathDisplayNamesMap["Videos"] = tr("Videos");
    m_systemPathDisplayNamesMap["Music"] = tr("Music");
    m_systemPathDisplayNamesMap["Pictures"] = tr("Pictures");
    m_systemPathDisplayNamesMap["Documents"] = tr("Documents");
    m_systemPathDisplayNamesMap["Downloads"] = tr("Downloads");
    m_systemPathDisplayNamesMap["Trash"] = tr("Trash");
    m_systemPathDisplayNamesMap["Disk"] = tr("Disk");
    m_systemPathDisplayNamesMap["Network"] = tr("Computers in LAN");
    m_systemPathDisplayNamesMap["UserShare"] = tr("My Shares");
    m_systemPathDisplayNamesMap["Computer"] = tr("Computer");


    m_systemPathIconNamesMap["Home"] = "folder-home";
    m_systemPathIconNamesMap["Desktop"] = "folder-desktop";
    m_systemPathIconNamesMap["Videos"] = "folder-videos";
    m_systemPathIconNamesMap["Music"] = "folder-music";
    m_systemPathIconNamesMap["Pictures"] = "folder-pictures";
    m_systemPathIconNamesMap["Documents"] = "folder-documents";
    m_systemPathIconNamesMap["Downloads"] = "folder-downloads";
}

QString PathManager::getSystemPath(QString key)
{
    if (m_systemPathsMap.isEmpty()){
        initPaths();
    }
    QString path = m_systemPathsMap.value(key);
    if(key == "Desktop" || key == "Videos" || key == "Music" ||
       key == "Pictures" || key == "Documents" || key == "Downloads" ||
       key == "Trash"){

        if (!QDir(path).exists()){
            bool flag = QDir::home().mkpath(path);
            qDebug() << "mkpath" << path << flag;
        }
    }
    return path;
}

QString PathManager::getSystemPathDisplayName(QString key)
{
    if (m_systemPathDisplayNamesMap.contains(key))
        return m_systemPathDisplayNamesMap.value(key);
    return QString();
}

QString PathManager::getSystemPathDisplayNameByPath(QString path)
{
    if (isSystemPath(path)){
        foreach (QString key, systemPathsMap().keys()) {
            if (systemPathsMap().value(key) == path){
                 return getSystemPathDisplayName(key);
            }
        }
    }
    return QString();
}


QString PathManager::getSystemPathIconName(QString key)
{
    if (m_systemPathIconNamesMap.contains(key))
        return m_systemPathIconNamesMap.value(key);
    return QString();
}

QString PathManager::getSystemPathIconNameByPath(const QString &path)
{
    if (isSystemPath(path)){
        foreach (QString key, systemPathsMap().keys()) {
            if (systemPathsMap().value(key) == path){
                 return getSystemPathIconName(key);
            }
        }
    }
    return QString();
}


QString PathManager::getSystemCachePath()
{
//    return QString("%1/%2").arg(StandardPath::getCachePath(), "systempath.json");
    return getCachePath("systempath");
}


void PathManager::loadSystemPaths()
{
    m_systemPathsMap["Home"] = StandardPath::getHomePath();
    m_systemPathsMap["Desktop"] = StandardPath::getDesktopPath();
    m_systemPathsMap["Videos"] = StandardPath::getVideosPath();
    m_systemPathsMap["Music"] = StandardPath::getMusicPath();
    m_systemPathsMap["Pictures"] = StandardPath::getPicturesPath();
    m_systemPathsMap["Documents"] = StandardPath::getDocumentsPath();
    m_systemPathsMap["Downloads"] = StandardPath::getDownloadsPath();
    m_systemPathsMap["Trash"] = StandardPath::getTrashFilesPath();
    m_systemPathsMap["Disk"] = StandardPath::getDiskPath();
    m_systemPathsMap["Network"] = StandardPath::getNetworkRootPath();
    m_systemPathsMap["UserShare"] = StandardPath::getUserShareRootPath();
    m_systemPathsMap["Computer"] = StandardPath::getComputerRootPath();

    m_systemPathsSet.reserve(m_systemPathsMap.size());

    foreach (QString key, m_systemPathsMap.keys()) {
        QString path = m_systemPathsMap.value(key);
        m_systemPathsSet << path;
        if(key == "Desktop" || key == "Videos" || key == "Music" ||
           key == "Pictures" || key == "Documents" || key == "Downloads" ||
           key == "Trash"){
            mkPath(path);
        }
    }
}

void PathManager::mkPath(const QString &path)
{
    if (!QDir(path).exists()){
        bool flag = QDir::home().mkpath(path);
        qDebug() << "mkpath" << path << flag;
    }
}

QMap<QString, QString> PathManager::systemPathDisplayNamesMap() const
{
    return m_systemPathDisplayNamesMap;
}

bool PathManager::isSystemPath(QString path) const
{
    return m_systemPathsSet.contains(path);
}

QMap<QString, QString> PathManager::systemPathsMap() const
{
    return m_systemPathsMap;
}


