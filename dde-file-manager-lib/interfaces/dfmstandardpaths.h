#ifndef DFMSTANDARDPATHS_H
#define DFMSTANDARDPATHS_H

#include <QString>
#include <QStandardPaths>

class DFMStandardPaths
{
public:
    enum StandardLocation {
        TrashPath,
        TrashFilesPath,
        TrashInfosPath,
        TranslationPath,
        TemplatesPath,
        MimeTypePath,
        PluginsPath,
        DbusFileDialogConfPath,
        ThumbnailPath,
        ThumbnailFailPath,
        ThumbnailLargePath,
        ThumbnailNormalPath,
        ThumbnailSmallPath,
        ApplicationConfigPath,  /* ~/.config */
        ApplicationSharePath,   /* /usr/share/dde-file-manager */
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
        ComputerRootPath
    };

    static QString standardLocation(StandardLocation type);

    static QString getConfigPath();
    static QString getCachePath();

private:
    DFMStandardPaths();
};

#endif // DFMSTANDARDPATHS_H
