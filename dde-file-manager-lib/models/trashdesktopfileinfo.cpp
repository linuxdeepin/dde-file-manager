#include "trashdesktopfileinfo.h"
#include <QIcon>
#include "interfaces/dfmstandardpaths.h"

TrashDesktopFileInfo::TrashDesktopFileInfo(const DUrl& fileUrl):
    DesktopFileInfo(fileUrl)
{
}

TrashDesktopFileInfo::~TrashDesktopFileInfo()
{

}

bool TrashDesktopFileInfo::canRename() const
{
    return false;
}

bool TrashDesktopFileInfo::canShare() const
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
    return DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::DesktopPath) + "/dde-trash.desktop");
}
