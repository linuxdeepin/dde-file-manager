#include "dfmstandardpaths.h"

#include <QDir>

QString DFMStandardPaths::standardLocation(DFMStandardPaths::StandardLocation type)
{
    switch (type) {
    case TrashPath:
        return QDir::homePath() + "/.local/share/Trash";
    case TrashFilesPath:
        return QDir::homePath() + "/.local/share/Trash/files";
    case TrashInfosPath:
        return QDir::homePath() + "/.local/share/Trash/info";
    case TranslationPath:
        return APPSHAREDIR"/translations";
    }

    return QString();
}

DFMStandardPaths::DFMStandardPaths()
{

}
