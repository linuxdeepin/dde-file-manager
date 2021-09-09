/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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
        TrashPath,                                  // 回收站路径
        TrashExpungedPath,                          // 回收站中文件源文件地址的位置
        TrashFilesPath,                             // 回收站文件存放地址
        TrashInfosPath,                             // 回收站文件信息路径
        TranslationPath,                            // 翻译文件所在路径
        TemplatesPath,                              // 临时文件所在路径
        MimeTypePath,                               // mimetype文件的路径
        PluginsPath,                                // 插件路径
        ThumbnailPath,                              // 缩略图路径
        ThumbnailFailPath,                          // 缩略图s失败路径
        ThumbnailLargePath,                         // 缩略图大图片路径
        ThumbnailNormalPath,                        // 缩略图普通大小路径
        ThumbnailSmallPath,                         // 缩略图小图片路径
        ApplicationConfigPath,                      // app配置文件路径 ~/.config
        ApplicationSharePath,                       // app共享路径 /usr/share/dde-file-manager
        RecentPath,                                 // 最近使用的文件保存路径
        HomePath,                                   // home目录的文件路径
        DesktopPath,                                // 桌面目录路径
        VideosPath,                                 // 视频目录路径
        MusicPath,                                  // 音乐目录路径
        PicturesPath,                               // 图片目录路径
        DocumentsPath,                              // 文档目录路径
        DownloadsPath,                              // 下载目录路径
        CachePath,                                  // 缓存文件路径 ~/.cache/dde-file-manager
        DiskPath,                                   // 磁盘路径
        NetworkRootPath,                            // 网络邻居路径
        UserShareRootPath,                          // 用户共享的根目录路径
        ComputerRootPath,                           // 计算机根目录路径
        Root,                                       // 根目录路径
        Vault                                       // 保险箱路径
    };

    static QString location(StandardLocation type);
    static QString iconName(StandardLocation type);
    static QString displayName(StandardLocation type);
    static QString fromStandardUrl(const QUrl &url);
    static QUrl toStandardUrl(const QString &localPath);
    static QString getCachePath();

private:
    StandardPaths();
};
DFMBASE_END_NAMESPACE

#endif // STANDARDPATHS_H
