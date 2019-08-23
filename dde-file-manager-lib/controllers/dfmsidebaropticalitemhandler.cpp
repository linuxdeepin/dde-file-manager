#include "dfmsidebaropticalitemhandler.h"

#include "interfaces/dfmleftsidebaritem.h"

DFMLeftSideBarItem *DFMSideBarOpticalItemHandler::createItem(const DUrl &url)
{
    QString displayName = "DVD";
    QString iconName("drive-harddisk-symbolic");

    DFMLeftSideBarItem * item = new DFMLeftSideBarItem(QIcon::fromTheme(iconName), displayName, url);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    item->setData(SIDEBAR_ID_OPTICAL, DFMLeftSideBarItem::ItemUseRegisteredHandlerRole);

    return item;
}

DFMSideBarOpticalItemHandler::DFMSideBarOpticalItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

void DFMSideBarOpticalItemHandler::cdAction(const DFMLeftSideBar *sidebar, const DFMLeftSideBarItem *item)
{
    return DFMSideBarItemInterface::cdAction(sidebar, item);
}

QMenu *DFMSideBarOpticalItemHandler::contextMenu(const DFMLeftSideBar *sidebar, const DFMLeftSideBarItem *item)
{
    return DFMSideBarItemInterface::contextMenu(sidebar, item);
}
