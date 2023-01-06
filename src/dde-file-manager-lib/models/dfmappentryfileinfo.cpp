// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmappentryfileinfo.h"
#include "dfmappentryfileinfo_p.h"
#include "dfmstandardpaths.h"

DFMAppEntryFileInfo::DFMAppEntryFileInfo(const DUrl &url)
    : DFMRootFileInfo (url),
      d_ptr(new DFMAppEntryFileInfoPrivate(this))
{
    QString desktopFilePath = DFMStandardPaths::location(DFMStandardPaths::ExtensionsAppEntryPath) + "/" + url.path();
    QDir filePath(desktopFilePath);
    if (!filePath.exists())
        desktopFilePath.append(".desktop");

    entryFile = DesktopFile(desktopFilePath);

    qDebug() << "appentry: " << url.path() << url;
}

QString DFMAppEntryFileInfo::fileDisplayName() const
{
    return entryFile.getDisplayName();
}

QString DFMAppEntryFileInfo::iconName() const
{
    return entryFile.getIcon();
}

bool DFMAppEntryFileInfo::exists() const
{
    if (!QFile(entryFile.getFileName()).exists())
        return false;
    if (entryFile.getDisplayName().isEmpty()) // empty name cause UI error.
        return false;
    auto binPath = executableBin();
    if (!QFile(binPath).exists()) {
        // if the executable is short cmd, combine it with $PATH and check again
        auto pathEnv = QString(qgetenv("PATH"));
        auto envs = pathEnv.split(":", QString::SkipEmptyParts);
        for (const auto &env: envs) {
            QFile executablePath(env + "/" + binPath);
            if (executablePath.exists())
                return true;
        }
        return false;
    }
    return true;
}

DAbstractFileInfo::FileType DFMAppEntryFileInfo::fileType() const
{
    return static_cast<FileType>(ItemType::AppEntry);
}

QVector<MenuAction> DFMAppEntryFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_UNUSED(type);
    return QVector<MenuAction>() << MenuAction::Open;
}

QString DFMAppEntryFileInfo::cmd() const
{
    // no parameters transfered
    static const QStringList unsupportedParams {
        "%U",   // A list of Urls
        "%u",   // A Url
        "%F",   // A list of files
        "%f"    // A file
    };
    auto cmd = entryFile.getExec();
    for (const auto &param: unsupportedParams)
        cmd.remove(param);
    return cmd.remove("\"").remove("'");
}

QString DFMAppEntryFileInfo::executableBin() const
{
    auto cmd = this->cmd();
    auto cmdArgs = cmd.split(" ", QString::SkipEmptyParts);
    auto bin = cmdArgs.count() > 0 ? cmdArgs.at(0) : QString();
    bin.remove("\"").remove("'");
    return bin;
}
