/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef STANDARDPATHS_H
#define STANDARDPATHS_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>

class QUrl;
class QString;
class QStandardPaths;
DFMBASE_BEGIN_NAMESPACE
class StandardPaths
{
public:
    enum StandardLocation {
        kTrashPath,   // 回收站路径
        kTrashExpungedPath,   // 回收站中文件源文件地址的位置
        kTrashFilesPath,   // 回收站文件存放地址
        kTrashInfosPath,   // 回收站文件信息路径
        kTranslationPath,   // 翻译文件所在路径
        kTemplatesPath,   // 临时文件所在路径
        kMimeTypePath,   // mimetype文件的路径
        kPluginsPath,   // 插件路径
        kExtensionsPath,
        kExtensionsAppEntryPath,
        kThumbnailPath,   // 缩略图路径
        kThumbnailFailPath,   // 缩略图s失败路径
        kThumbnailLargePath,   // 缩略图大图片路径
        kThumbnailNormalPath,   // 缩略图普通大小路径
        kThumbnailSmallPath,   // 缩略图小图片路径
        kApplicationConfigPath,   // app配置文件路径 ~/.config
        kApplicationSharePath,   // app共享路径 /usr/share/dde-file-manager
        kRecentPath,   // 最近使用的文件保存路径
        kHomePath,   // home目录的文件路径
        kDesktopPath,   // 桌面目录路径
        kVideosPath,   // 视频目录路径
        kMusicPath,   // 音乐目录路径
        kPicturesPath,   // 图片目录路径
        kDocumentsPath,   // 文档目录路径
        kDownloadsPath,   // 下载目录路径
        kCachePath,   // 缓存文件路径 ~/.cache/dde-file-manager
        kDiskPath,   // 磁盘路径
        kNetworkRootPath,   // 网络邻居路径
        kUserShareRootPath,   // 用户共享的根目录路径
        kComputerRootPath,   // 计算机根目录路径
        kRoot,   // 根目录路径
        kVault   // 保险箱路径
    };

    static QString location(StandardLocation type);
    static QString location(const QString &dirName);
    static QString iconName(StandardLocation type);
    static QString iconName(const QString &dirName);
    static QString displayName(const QString &dirName);
    static QString displayName(StandardLocation type);
    static QString fromStandardUrl(const QUrl &url);
    static QUrl toStandardUrl(const QString &localPath);
    static QString getCachePath();

private:
    StandardPaths();
};
DFMBASE_END_NAMESPACE

#endif   // STANDARDPATHS_H
