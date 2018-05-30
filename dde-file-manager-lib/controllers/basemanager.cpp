/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    return QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::CachePath), QString("%1.json").arg(fileBaseName));
}

QString BaseManager::getConfigPath(const QString &fileBaseName)
{
    return QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::ApplicationConfigPath), QString("%1.json").arg(fileBaseName));
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


