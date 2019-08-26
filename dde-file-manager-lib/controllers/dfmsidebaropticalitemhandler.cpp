#include "dfmsidebaropticalitemhandler.h"

#include "app/define.h"
#include "interfaces/dfmsidebaritem.h"
#include "disomaster.h"

DFM_BEGIN_NAMESPACE

DFMSideBarItem *DFMSideBarOpticalItemHandler::createItem(const DUrl &url)
{
    QString displayName = "DVD";
    QString iconName("drive-harddisk-symbolic");

    DFMSideBarItem * item = new DFMSideBarItem(QIcon::fromTheme(iconName), displayName, url);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    item->setData(SIDEBAR_ID_OPTICAL, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    return item;
}

DFMSideBarOpticalItemHandler::DFMSideBarOpticalItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

void DFMSideBarOpticalItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    if (ISOMaster->currentDevice() == item->url().path()) {
        return;
    }

    return DFMSideBarItemInterface::cdAction(sidebar, item);
}

QMenu *DFMSideBarOpticalItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    return DFMSideBarItemInterface::contextMenu(sidebar, item);
}

DFM_END_NAMESPACE
