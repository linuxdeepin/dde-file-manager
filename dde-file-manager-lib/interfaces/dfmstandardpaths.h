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
        ApplicationConfigPath
    };

    static QString standardLocation(StandardLocation type);

    static QString getConfigPath();

private:
    DFMStandardPaths();
};

#endif // DFMSTANDARDPATHS_H
