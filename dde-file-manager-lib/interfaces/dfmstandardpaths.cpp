#include "dfmstandardpaths.h"

#include <QDir>
#include <QApplication>
#include <QStandardPaths>

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
    case ApplicationConfigPath:
        return getConfigPath();
    case ThumbnailPath:
        return QDir::homePath() + "/.cache/thumbnails";
    case ThumbnailFailPath:
        return standardLocation(ThumbnailPath) + "/fail";
    case ThumbnailLargePath:
        return standardLocation(ThumbnailPath) + "/large";
    case ThumbnailNormalPath:
        return standardLocation(ThumbnailPath) + "/normal";
    case ThumbnailSmallPath:
        return standardLocation(ThumbnailPath) + "/small";
    }

    return QString();
}

QString DFMStandardPaths::getConfigPath()
{
    QString projectName = QMAKE_TARGET;
    QDir::home().mkpath(".config");
    QDir::home().mkpath(QString("%1/%2/").arg(".config", projectName));
    QString defaultPath = QString("%1/%2/%3").arg(QDir::homePath(), ".config", projectName);
    return defaultPath;
}

DFMStandardPaths::DFMStandardPaths()
{

}
