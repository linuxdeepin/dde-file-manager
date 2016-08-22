#include "basemanager.h"
#include "../shutil/standardpath.h"
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

QString BaseManager::getCachePath(const QString &key)
{
    return QString("%1/%2").arg(StandardPath::getCachePath(), QString("%1.json").arg(key));
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


