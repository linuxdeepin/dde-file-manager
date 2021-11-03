/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "usershare/usersharemanager.h"
#include "models/dfmrootfileinfo.h"
#include "dfmapplication.h"
#include "dfmsettings.h"
#include "interfaces/dfmstandardpaths.h"
#include "interfaces/dfmglobal.h"
#include "plugins/schemepluginmanager.h" //NOTE [REN] 添加依赖头文件

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

    //NOTE [REN] 将PLUGIN加载到m_systemPathDisplayNamesMap
    for (auto plugin : SchemePluginManager::instance()->schemePlugins()) {
        DFMSideBarItem *item = plugin.second->createSideBarItem();
        m_systemPathDisplayNamesMap[plugin.first] = item->text();
    }

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
                QString displayName;
                const QString &name = getSystemPathDisplayName(key);
                // 系统盘如果有别名，就以别名显示
                displayName = (path == "/" ? getSystemPathDisplayAliasByName(name) : name);
                if (displayName.isEmpty())
                    displayName = name;
                return displayName;
            }
        }
    }
    return QString();
}

QString PathManager::getSystemPathDisplayAliasByName(const QString &name)
{
    const QVariantList &list = DFMApplication::genericSetting()->value(DISKALIAS_GROUP, DISKALIAS_ITEMS).toList();
    QString alias;

    for (const QVariant &v : list) {
        const QVariantMap &map = v.toMap();
        if (map.value(DISKALIAS_ITEM_NAME).toString() == name) {
            alias = map.value(DISKALIAS_ITEM_ALIAS).toString();
            break;
        }
    }

    return alias;
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

/**
 * @brief 系统盘&数据盘路径
 * @return
 */
QStringList PathManager::getSystemDiskAndDataDiskPathGroup()
{
    const QString &userName = UserShareManager::getCurrentUserName();
    QStringList group {"/", "/data", QString("/home/%1").arg(userName), QString("/data/home/%1").arg(userName)};
    return group;
}

/**
 * @brief 指定的挂载点
 * @return
 */
QStringList PathManager::getMountRangePathGroup()
{
    const QString &userName = UserShareManager::getCurrentUserName();
    QStringList ranges {
        QString("/mnt"),
        QString("/home/%1").arg(userName),
        QString("/media/%1").arg(userName),
        QString("/data/home/%1").arg(userName),
    };
    return ranges;
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

    //NOTE [REN] 将PLUGIN加载到m_systemPathsMap
    for (auto plugin : SchemePluginManager::instance()->schemePlugins()) {
        DFMSideBarItem *item = plugin.second->createSideBarItem();
        m_systemPathsMap[plugin.first] = item->url().toString();
    }

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

/**
 * sp3 feature: 计算机页面展示分区范围说明
 * 分区识别范围：
 * 1. 系统指定的分区
 *  /，显示为系统盘
 *  /data，显示为数据盘
 *  /home/username/
 *  /media/username/
 *  /mnt/
 * 2. 除上述之外的挂载分区，不在系统文管中展示；
 *
 * @return 返回 true 则计算机页面显示该分区
 */
bool PathManager::isVisiblePartitionPath(const DAbstractFileInfoPointer &fi)
{
#ifdef SP3_UNSTABLE_FEATURE_ENABLE
    QString suffix = fi->suffix();
    if (DFMGlobal::isRootUser()) { // root 用户不受限
        return true;
    } else if (suffix == SUFFIX_USRDIR || suffix == SUFFIX_GVFSMP) { // 系统路径，MTP等不受限
        return true;
    } else { // localdisk
        DUrl url = fi->redirectedFileUrl();
        DUrl parentUrl = url.parentUrl();
        const QString &path = url.toLocalFile();
        const QString &parentPath = parentUrl.toLocalFile();

        // [0] 光驱
        if (isOptical(fi))
            return true;

        // [1] 当打开文管时挂载U盘，进入到此函数时，很可能还没挂载完成,获取不到path
        // 为了避免这种情况下无法显示，因此直接返回true
        // todo：此处可能会被报bug，但目前没有想到更好的方案，待优化
        if (path.isEmpty())
            return true;

        // [2] 排除系统盘 & 数据盘
        const QStringList &sysRanges = getSystemDiskAndDataDiskPathGroup();
        if (sysRanges.contains(path))
            return true;

        // [3] 挂载只显示挂载到以下挂载点的内容
        const QStringList &parentRanges = getMountRangePathGroup();
        if (parentRanges.contains(parentPath))
            return true;
    }
    qDebug() << "partition ignore path:" << fi->redirectedFileUrl().toLocalFile();

    return false;
#else
    Q_UNUSED(fi)
    return true;
#endif
}

bool PathManager::isOptical(const DAbstractFileInfoPointer &fi)
{
    QString path = fi->fileUrl().toString();
    return path.contains("dfmroot:///sr") ? true : false;
}

QMap<QString, QString> PathManager::systemPathsMap() const
{
    return m_systemPathsMap;
}


