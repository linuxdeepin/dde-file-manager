#include "recentfileinfo.h"

#include "../app/global.h"

#include "../shutil/iconprovider.h"

RecentFileInfo::RecentFileInfo()
    : AbstractFileInfo()
{

}

RecentFileInfo::RecentFileInfo(const DUrl &url)
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
    return true;
}

QIcon RecentFileInfo::fileIcon() const
{
    return fileIconProvider->getFileIcon(absoluteFilePath());
}

QVector<AbstractFileInfo::MenuAction> RecentFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys << MenuAction::ClearRecent
                   << MenuAction::Separator
                   << MenuAction::DisplayAs
                   << MenuAction::SortBy
                   << MenuAction::Separator
                   << MenuAction::Property;

        QMap<MenuAction, QVector<MenuAction> > subMenu;
        QVector<MenuAction> subActionKeys;

        subActionKeys << MenuAction::IconView << MenuAction::ListView;

        subMenu[MenuAction::DisplayAs] = subActionKeys;
        subActionKeys.clear();

        subActionKeys << MenuAction::Name << MenuAction::Size << MenuAction::Type
                      << MenuAction::CreatedDate << MenuAction::LastModifiedDate;

        subMenu[MenuAction::SortBy] = subActionKeys;
    } else if (type == SingleFile){

        actionKeys << MenuAction::Open << MenuAction::OpenWith
                   << MenuAction::OpenFileLocation
                   << MenuAction::Separator
                   << MenuAction::Remove
                   << MenuAction::Separator
                   << MenuAction::Property;
    }else if (type == MultiFiles){

        actionKeys << MenuAction::Open
                   << MenuAction::OpenFileLocation
                   << MenuAction::Separator
                   << MenuAction::Remove
                   << MenuAction::Separator
                   << MenuAction::Property;
    }

    return actionKeys;
}

DUrl RecentFileInfo::parentUrl() const
{
    return DUrl::fromRecentFile("/");
}

QString RecentFileInfo::lastOpenedDisplayName() const
{
    return m_lastOpened.toString(timeFormat());
}

QDateTime RecentFileInfo::lastOpened() const
{
    return m_lastOpened;
}

void RecentFileInfo::setLastOpened(const QDateTime &lastOpened)
{
    m_lastOpened = lastOpened;
}


