#include "pathmanager.h"
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QDebug>
#include <QDir>
#include "interfaces/dfmstandardpaths.h"

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
    m_systemPathDisplayNamesMap["System Disk"] = tr("System Disk");
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
    return getConfigPath("systempath");
}


void PathManager::loadSystemPaths()
{
    m_systemPathsMap["Home"] = DFMStandardPaths::standardLocation(DFMStandardPaths::HomePath);
    m_systemPathsMap["Desktop"] = DFMStandardPaths::standardLocation(DFMStandardPaths::DesktopPath);
    m_systemPathsMap["Videos"] = DFMStandardPaths::standardLocation(DFMStandardPaths::VideosPath);
    m_systemPathsMap["Music"] = DFMStandardPaths::standardLocation(DFMStandardPaths::MusicPath);
    m_systemPathsMap["Pictures"] = DFMStandardPaths::standardLocation(DFMStandardPaths::PicturesPath);
    m_systemPathsMap["Documents"] = DFMStandardPaths::standardLocation(DFMStandardPaths::DocumentsPath);
    m_systemPathsMap["Downloads"] = DFMStandardPaths::standardLocation(DFMStandardPaths::DownloadsPath);
    m_systemPathsMap["Trash"] = DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath);
    m_systemPathsMap["System Disk"] = DFMStandardPaths::standardLocation(DFMStandardPaths::DiskPath);
    m_systemPathsMap["Network"] = DFMStandardPaths::standardLocation(DFMStandardPaths::NetworkRootPath);
    m_systemPathsMap["UserShare"] = DFMStandardPaths::standardLocation(DFMStandardPaths::UserShareRootPath);
    m_systemPathsMap["Computer"] = DFMStandardPaths::standardLocation(DFMStandardPaths::ComputerRootPath);

    m_systemPathsSet.reserve(m_systemPathsMap.size());

    foreach (const QString &key, m_systemPathsMap.keys()) {
        const QString &path = m_systemPathsMap.value(key);

        if (key != "Trash")
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


