#include "recentfileinfo.h"

#include "../app/global.h"

#include "../shutil/iconprovider.h"

RecentFileInfo::RecentFileInfo()
    : AbstractFileInfo()
{

}

RecentFileInfo::RecentFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{

}

RecentFileInfo::RecentFileInfo(const QString &url)
    : AbstractFileInfo(url)
{

}

bool RecentFileInfo::isCanRename() const
{
    return false;
}

bool RecentFileInfo::isWritable() const
{
    return false;
}

bool RecentFileInfo::isReadable() const
{
    if(filePath() == "/")
        return true;

    return AbstractFileInfo::isReadable();
}

bool RecentFileInfo::isDir() const
{
    return filePath() == "/";
}

QIcon RecentFileInfo::fileIcon() const
{
    return iconProvider->getFileIcon(absoluteFilePath());
}
