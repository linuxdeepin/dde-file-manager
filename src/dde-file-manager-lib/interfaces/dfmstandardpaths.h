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

#ifndef DFMSTANDARDPATHS_H
#define DFMSTANDARDPATHS_H

#include <QString>
#include <QStandardPaths>

class DUrl;
class DFMStandardPaths
{
public:
    enum StandardLocation {
        TrashPath,
        TrashExpungedPath,
        TrashFilesPath,
        TrashInfosPath,
        TranslationPath,
        TemplatesPath,
        MimeTypePath,
        PluginsPath,
        ThumbnailPath,
        ThumbnailFailPath,
        ThumbnailLargePath,
        ThumbnailNormalPath,
        ThumbnailSmallPath,
        ApplicationConfigPath,  /* ~/.config */
        ApplicationSharePath,   /* /usr/share/dde-file-manager */
        RecentPath,
        HomePath,
        DesktopPath,
        VideosPath,
        MusicPath,
        PicturesPath,
        DocumentsPath,
        DownloadsPath,
        CachePath,              /* ~/.cache/dde-file-manager */
        DiskPath,
        NetworkRootPath,
        UserShareRootPath,
        ComputerRootPath,
        Root,
        Vault
    };

    static QString location(StandardLocation type);
    static QString fromStandardUrl(const DUrl &standardUrl);
    static DUrl toStandardUrl(const QString &localPath);

#ifdef QMAKE_TARGET
    static QString getConfigPath();
#endif
    static QString getCachePath();

private:
    DFMStandardPaths();
};

#endif // DFMSTANDARDPATHS_H
