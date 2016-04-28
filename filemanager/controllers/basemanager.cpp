#include "basemanager.h"
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


