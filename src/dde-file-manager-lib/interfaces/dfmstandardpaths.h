// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
        ExtensionsPath,
        ExtensionsAppEntryPath,
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
