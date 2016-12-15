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

bool ComputerDesktopFileInfo::isCanRename() const
{
    return false;
}

bool ComputerDesktopFileInfo::isCanShare() const
{
    return false;
}

bool ComputerDesktopFileInfo::isWritable() const
{
    return false;
}

QVector<MenuAction> ComputerDesktopFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    return QVector<MenuAction> ();
}

QList<QIcon> ComputerDesktopFileInfo::additionalIcon() const
{
    return QList<QIcon> ();
}

DUrl ComputerDesktopFileInfo::computerDesktopFileUrl()
{
    return DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::DesktopPath) + "/dde-computer.desktop");
}
