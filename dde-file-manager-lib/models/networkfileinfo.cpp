#include "networkfileinfo.h"

#include "app/define.h"

#include "singleton.h"

#include "controllers/pathmanager.h"

#include "views/dfileview.h"

#include <QIcon>

NetworkFileInfo::NetworkFileInfo(const DUrl &url):
    DAbstractFileInfo(url)
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

bool NetworkFileInfo::exists() const
{
    return true;
}

bool NetworkFileInfo::isReadable() const
{
    return true;
}

bool NetworkFileInfo::isWritable() const
{
    return true;
}

bool NetworkFileInfo::canRename() const
{
    return false;
}

//QIcon NetworkFileInfo::fileIcon() const
//{
//    return QIcon(fileIconProvider->getThemeIconPath(mimeTypeName(), 256));
//}

bool NetworkFileInfo::isDir() const
{
    return true;
}

DUrl NetworkFileInfo::parentUrl() const
{
    return DUrl();
}

QString NetworkFileInfo::fileDisplayName() const
{

    if (systemPathManager->isSystemPath(fileUrl().toString()))
        return systemPathManager->getSystemPathDisplayNameByPath(fileUrl().toString());

    return m_networkNode.displayName();
}

//QString NetworkFileInfo::mimeTypeName(QMimeDatabase::MatchMode mode) const
//{
//    Q_UNUSED(mode)

//    return m_networkNode.iconType();
//}
NetworkNode NetworkFileInfo::networkNode() const
{
    return m_networkNode;
}

void NetworkFileInfo::setNetworkNode(const NetworkNode &networkNode)
{
    m_networkNode = networkNode;
}

QVector<MenuAction> NetworkFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
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

QList<DAbstractFileInfo::SelectionMode> NetworkFileInfo::supportSelectionModes() const
{
    return QList<SelectionMode>() << SingleSelection;
}

Qt::ItemFlags NetworkFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

quint8 NetworkFileInfo::supportViewMode() const
{
    return DFileView::IconMode;
}

QString NetworkFileInfo::iconName() const
{
    return m_networkNode.iconType();
}
