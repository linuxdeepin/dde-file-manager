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

#include "pathmanager.h"
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QDebug>
#include <QDir>

#include "controllers/vaultcontroller.h"

#include "dfmapplication.h"
#include "dfmsettings.h"

#include "interfaces/dfmstandardpaths.h"

DFM_USE_NAMESPACE

PathManager::PathManager(QObject *parent)
    : QObject(parent)
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
    m_systemPathDisplayNamesMap["Recent"] = tr("Recent");
    m_systemPathDisplayNamesMap["Vault"] = tr("File Vault");

    if (DFMApplication::instance()->genericObtuselySetting()->value("Disk/Options", "windowsStyle").toBool()) {
        m_systemPathDisplayNamesMap["System Disk"] = m_systemPathDisplayNamesMap["System Disk"].append(" (C:)");
    }

    m_systemPathIconNamesMap["Recent"] = "document-open-recent";
    m_systemPathIconNamesMap["Home"] = "user-home";
    m_systemPathIconNamesMap["Desktop"] = "user-desktop";
    m_systemPathIconNamesMap["Videos"] = "folder-videos";
    m_systemPathIconNamesMap["Music"] = "folder-music";
    m_systemPathIconNamesMap["Pictures"] = "folder-pictures";
    m_systemPathIconNamesMap["Documents"] = "folder-documents";
    m_systemPathIconNamesMap["Downloads"] = "folder-downloads";
    m_systemPathIconNamesMap["Trash"] = "user-trash";
    m_systemPathIconNamesMap["Computer"] = "computer";
    m_systemPathIconNamesMap["System Disk"] = "drive-harddisk-root";
    m_systemPathIconNamesMap["Network"] = "network-server"; // folder-remote ?
    m_systemPathIconNamesMap["UserShare"] = "folder-publicshare";
    m_systemPathIconNamesMap["Vault"] = "drive-harddisk-encrypted"; // 保险柜,图标还未确定
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

QString PathManager::getSystemPathDisplayName(QString key) const
{
    if (m_systemPathDisplayNamesMap.contains(key))
        return m_systemPathDisplayNamesMap.value(key);

    return QString();
}

void cleanPath(QString &path)
{
    //这里去掉/data的目的 是让通过数据盘路径进入的用户目录下的Docunment,Vedios等文件也可以被翻译
    if (path.startsWith("/data"))
    {
        path.remove(0, sizeof("/data") - 1);
    }

    if (path.size() > 1 && path.at(0) == '/' && path.endsWith("/")) {
        path.chop(1);
    }
}

QString PathManager::getSystemPathDisplayNameByPath(QString path)
{
    cleanPath(path);

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

QString PathManager::getSystemPathIconNameByPath(QString path)
{
    cleanPath(path);

    if (isSystemPath(path)){
        foreach (QString key, systemPathsMap().keys()) {
            if (systemPathsMap().value(key) == path){
                 return getSystemPathIconName(key);
            }
        }
    }
    return QString();
}

void PathManager::loadSystemPaths()
{
    m_systemPathsMap["Home"] = DFMStandardPaths::location(DFMStandardPaths::HomePath);
    m_systemPathsMap["Desktop"] = DFMStandardPaths::location(DFMStandardPaths::DesktopPath);
    m_systemPathsMap["Videos"] = DFMStandardPaths::location(DFMStandardPaths::VideosPath);
    m_systemPathsMap["Music"] = DFMStandardPaths::location(DFMStandardPaths::MusicPath);
    m_systemPathsMap["Pictures"] = DFMStandardPaths::location(DFMStandardPaths::PicturesPath);
    m_systemPathsMap["Documents"] = DFMStandardPaths::location(DFMStandardPaths::DocumentsPath);
    m_systemPathsMap["Downloads"] = DFMStandardPaths::location(DFMStandardPaths::DownloadsPath);
    m_systemPathsMap["Trash"] = DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath);
    m_systemPathsMap["System Disk"] = DFMStandardPaths::location(DFMStandardPaths::DiskPath);
    m_systemPathsMap["Network"] = DFMStandardPaths::location(DFMStandardPaths::NetworkRootPath);
    m_systemPathsMap["UserShare"] = DFMStandardPaths::location(DFMStandardPaths::UserShareRootPath);
    m_systemPathsMap["Computer"] = DFMStandardPaths::location(DFMStandardPaths::ComputerRootPath);
    m_systemPathsMap["Recent"] = DFMStandardPaths::location(DFMStandardPaths::RecentPath);
    m_systemPathsMap["Vault"] = DFMStandardPaths::location(DFMStandardPaths::Vault); // 保险库路径

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
    cleanPath(path);

    return m_systemPathsSet.contains(path);
}

QMap<QString, QString> PathManager::systemPathsMap() const
{
    return m_systemPathsMap;
}


