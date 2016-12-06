#include "trashdesktopfileinfo.h"
#include <QIcon>
#include "shutil/standardpath.h"

TrashDesktopFileInfo::TrashDesktopFileInfo(const DUrl& fileUrl):
    DesktopFileInfo(fileUrl)
{
}

TrashDesktopFileInfo::~TrashDesktopFileInfo()
{

}

bool TrashDesktopFileInfo::isCanRename() const
{
    return false;
}

bool TrashDesktopFileInfo::isCanShare() const
{
    return false;
}

bool TrashDesktopFileInfo::isWritable() const
{
    return false;
}

QVector<MenuAction> TrashDesktopFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actions;
    actions << MenuAction::Open
    << MenuAction::Separator
    << MenuAction::Property;

    return actions;
}

QList<QIcon> TrashDesktopFileInfo::additionalIcon() const
{
    return QList<QIcon> ();
}

DUrl TrashDesktopFileInfo::trashDesktopFileUrl()
{
    return DUrl::fromLocalFile(StandardPath::getDesktopPath() + "/dde-trash.desktop");
}
