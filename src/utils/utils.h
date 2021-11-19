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

#ifndef UTILS_H
#define UTILS_H

#include <QtCore>
#include <QPixmap>

#define desktopLocation QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).at(0)
#define RichDirPrefix ".deepin_rich_dir_"
#define AppSuffix ".desktop"

#define ComputerUrl "computer://"
#define TrashUrl "trash://"
#define FilePrefix "file://"

const QSet<QString> SupportMimeTypes{"image/png" ,"image/jpeg" , "image/gif"
                                      , "image/bmp", "image/tiff" , "image/svg+xml"
                                      , "application/pdf"
                                    };
QString getThumbnailsPath();
QString decodeUrl(QString url);
QString deleteFilePrefix(QString path);
bool isDesktop(QString url);
bool isAppGroup(QString url);
bool isApp(QString url);
bool isAllApp(QStringList urls);
bool isComputer(QString url);
bool isTrash(QString url);
bool isFolder(QString url);
bool isInDesktop(QString url);
bool isInDesktopFolder(QString url);
bool isDesktopAppFile(QString url);
bool isAvfsMounted();

bool isRequestThumbnail(QString url);
QString getMimeTypeGenericIconName(QString url);
QString getMimeTypeIconName(QString url);
QString getMimeTypeName(QString url);


QString getQssFromFile(QString filename);
QString joinPath(const QString &path, const QString& fileName);
QByteArray joinPath(const QByteArray &path, const QByteArray& fileName);

QPixmap svgToPixmap(const QString& path, int w, int h);
QPixmap svgToHDPIPixmap(const QString& path);
QPixmap svgToHDPIPixmap(const QString& path, int w, int h);

#define REMOTE_KEY          "key"
#define REMOTE_PROTOCOL     "protocol"
#define REMOTE_HOST         "host"
#define REMOTE_SHARE        "share"
#define REMOTE_DISPLAYNAME  "name"
#define REMOTE_ANONYMOUS    "anonymous"
#define REMOTE_USERNAME     "username"
#define REMOTE_PASSWORD     "password"

#define CONFIG_PATH         (QDir::homePath() + "/.config/deepin/dde-file-manager.json")

/*!
 * \brief This class is used for managing the stashed remote mounts configurations in DFM config file
 */
class RemoteMountsStashManager {
public:
    static QList<QVariantMap> remoteMounts();
    static void stashRemoteMount(const QString &mpt, const QString &displayName);
    static void removeRemoteMountItem(const QString &key);
    static void clearRemoteMounts();
    static void stashCurrentMounts();
    static QString getDisplayNameByConnUrl(const QString &url);
    static QString normalizeConnUrl(const QString &url);
};


void clearStageDir(const QString &stagingRoot);
#endif // UTILS_H

