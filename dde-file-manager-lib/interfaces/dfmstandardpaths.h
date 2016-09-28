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
        TranslationPath
    };

    static QString standardLocation(StandardLocation type);

private:
    DFMStandardPaths();
};

#endif // DFMSTANDARDPATHS_H
