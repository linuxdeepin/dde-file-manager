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
    if (m_systemPaths.isEmpty()){
        saveSystemPaths();
    }
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
    QString path = getSystemPath(key);
    return QDir(path).dirName();
}

QString PathManager::getSystemCachePath()
{
    return QString("%1/%2").arg(StandardPath::getCachePath(), "systempath.json");
}

void PathManager::saveSystemPaths()
{
    m_systemPaths["Desktop"] = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0);
    m_systemPaths["Videos"] = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).at(0);
    m_systemPaths["Musics"] = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).at(0);
    m_systemPaths["Pictures"] = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
    m_systemPaths["Documents"] = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    m_systemPaths["Download"] = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).at(0);

    QVariantMap pathCache;
    foreach (QString key, m_systemPaths.keys()) {
        QString path = m_systemPaths.value(key);
        pathCache.insert(key, path);
        mkPath(path);
        m_fileSystemWatcher->addPath(path);
    }
    QJsonDocument doc(QJsonObject::fromVariantMap(pathCache));
    writeCacheToFile(getSystemCachePath(), doc.toJson());

}

void PathManager::loadSystemPaths()
{
    QString cacheContent = readCacheFromFile(getSystemCachePath());
    if (!cacheContent.isEmpty()){
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(cacheContent.toLocal8Bit(),&error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject obj = doc.object();
            foreach (QString key, obj.keys()) {
                QString path = obj.value(key).toString();
                m_systemPaths.insert(key, path);
                mkPath(path);
                m_fileSystemWatcher->addPath(path);
            }
        }else{
            qDebug() << "load cache file: " << getSystemCachePath() << error.errorString();
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

void PathManager::handleDirectoryChanged(const QString &path)
{
    qDebug() << path;
    mkPath(path);
    m_fileSystemWatcher->addPath(path);
}

QMap<QString, QString> PathManager::systemPaths() const
{
    return m_systemPaths;
}


