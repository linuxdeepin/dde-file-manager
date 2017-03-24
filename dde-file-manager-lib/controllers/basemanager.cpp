#include "basemanager.h"
#include "interfaces/dfmstandardpaths.h"
#include <QFile>
#include <QDebug>

BaseManager::BaseManager()
{

}

BaseManager::~BaseManager()
{

}

void BaseManager::load()
{

}

void BaseManager::save()
{

}

QString BaseManager::getCachePath(const QString &fileBaseName)
{
    return QString("%1/%2").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::CachePath), QString("%1.json").arg(fileBaseName));
}

QString BaseManager::getConfigPath(const QString &fileBaseName)
{
    return QString("%1/%2").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationConfigPath), QString("%1.json").arg(fileBaseName));
}

void BaseManager::writeCacheToFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QFile::WriteOnly)){
        file.write(content.toLocal8Bit());
    }
    file.close();
}

QString BaseManager::readCacheFromFile(const QString &path)
{
    qDebug() << path;
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
    {
        qDebug() << path << "isn't exists!";
        return QString();
    }
    QByteArray content = file.readAll();
    file.close();
    return QString(content);
}


