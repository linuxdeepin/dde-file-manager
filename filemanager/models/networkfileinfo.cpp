#include "networkfileinfo.h"
#include "../app/global.h"
#include "../shutil/iconprovider.h"
#include <QIcon>

NetworkFileInfo::NetworkFileInfo():
    AbstractFileInfo()
{

}

NetworkFileInfo::NetworkFileInfo(const DUrl &url):
    AbstractFileInfo(url)
{

}

NetworkFileInfo::~NetworkFileInfo()
{

}

QString NetworkFileInfo::filePath() const
{
    return m_networkNode.url();
}

QString NetworkFileInfo::absoluteFilePath() const
{
    return m_networkNode.url();
}

bool NetworkFileInfo::isCanRename() const
{
    return false;
}

QIcon NetworkFileInfo::fileIcon() const
{
    return QIcon(fileIconProvider->getThemeIconPath(mimeTypeName(), 256));
}

bool NetworkFileInfo::isDir() const
{
    return true;
}

DUrl NetworkFileInfo::parentUrl() const
{
    return DUrl();
}

QString NetworkFileInfo::displayName() const
{
    return m_networkNode.displayName();
}

QString NetworkFileInfo::mimeTypeName() const
{
    return m_networkNode.iconType();
}
NetworkNode NetworkFileInfo::networkNode() const
{
    return m_networkNode;
}

void NetworkFileInfo::setNetworkNode(const NetworkNode &networkNode)
{
    m_networkNode = networkNode;
}

QVector<MenuAction> NetworkFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;
    if(type == SpaceArea) {

    } else if (type == SingleFile){
        actionKeys << MenuAction::Open;
        actionKeys << MenuAction::OpenInNewWindow;
    }else if (type == MultiFiles){

    }
    return actionKeys;
}

QAbstractItemView::SelectionMode NetworkFileInfo::supportSelectionMode() const
{
    return QAbstractItemView::SingleSelection;
}

Qt::ItemFlags NetworkFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

