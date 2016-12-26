#ifndef DFMSTANDARDPATHS_H
#define DFMSTANDARDPATHS_H

#include <QString>

class DFMStandardPaths
{
public:
    enum StandardLocation {
        TrashPath,
        TrashFilesPath,
        TrashInfosPath,
        TranslationPath,
        ApplicationConfigPath,
        ThumbnailPath,
        ThumbnailFailPath,
        ThumbnailLargePath,
        ThumbnailNormalPath,
        ThumbnailSmallPath
    };

    static QString standardLocation(StandardLocation type);

    static QString getConfigPath();

private:
    DFMStandardPaths();
};

#endif // DFMSTANDARDPATHS_H
