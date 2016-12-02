#include "dfmstandardpaths.h"

#include <QDir>
#include <QApplication>

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
