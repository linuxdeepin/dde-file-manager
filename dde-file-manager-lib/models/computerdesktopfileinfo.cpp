#include "computerdesktopfileinfo.h"
#include <QIcon>
#include "interfaces/dfmstandardpaths.h"

ComputerDesktopFileInfo::ComputerDesktopFileInfo(const DUrl &fileUrl):
    DesktopFileInfo(fileUrl)
{

}

ComputerDesktopFileInfo::~ComputerDesktopFileInfo()
{

}

bool ComputerDesktopFileInfo::canRename() const
{
    return false;
}

bool ComputerDesktopFileInfo::canShare() const
{
    return false;
}

bool ComputerDesktopFileInfo::isWritable() const
{
    return false;
}

QVector<MenuAction> ComputerDesktopFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_UNUSED(type);
    QVector<MenuAction> actions;
    actions << MenuAction::Open
    << MenuAction::Separator;

    if(type == SingleFile)
        actions << MenuAction::CreateSymlink;

    actions << MenuAction::Property;

    return actions;
}

QList<QIcon> ComputerDesktopFileInfo::additionalIcon() const
{
    return QList<QIcon> ();
}

DUrl ComputerDesktopFileInfo::computerDesktopFileUrl()
{
    return DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::DesktopPath) + "/dde-computer.desktop");
}
