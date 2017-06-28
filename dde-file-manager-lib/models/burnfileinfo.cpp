#include "burnfileinfo.h"

#include "app/define.h"

#include "singleton.h"

#include "controllers/pathmanager.h"

#include "views/dfileview.h"

#include <QIcon>

BurnFileInfo::BurnFileInfo(const DUrl &url):
    DAbstractFileInfo(url)
{

}

BurnFileInfo::~BurnFileInfo()
{

}

QString BurnFileInfo::filePath() const
{
    return m_networkNode.url();
}

QString BurnFileInfo::absoluteFilePath() const
{
    return m_networkNode.url();
}

bool BurnFileInfo::exists() const
{
    return true;
}

bool BurnFileInfo::isReadable() const
{
    return true;
}

bool BurnFileInfo::isWritable() const
{
    return true;
}

bool BurnFileInfo::canRename() const
{
    return false;
}

//QIcon BurnFileInfo::fileIcon() const
//{
//    return QIcon(fileIconProvider->getThemeIconPath(mimeTypeName(), 256));
//}

bool BurnFileInfo::isDir() const
{
    return true;
}

DUrl BurnFileInfo::parentUrl() const
{
    return DUrl();
}

QString BurnFileInfo::fileDisplayName() const
{

    if (systemPathManager->isSystemPath(fileUrl().toString()))
        return systemPathManager->getSystemPathDisplayNameByPath(fileUrl().toString());

    return m_networkNode.displayName();
}

//QString BurnFileInfo::mimeTypeName(QMimeDatabase::MatchMode mode) const
//{
//    Q_UNUSED(mode)

//    return m_networkNode.iconType();
//}
NetworkNode BurnFileInfo::networkNode() const
{
    return m_networkNode;
}

void BurnFileInfo::setNetworkNode(const NetworkNode &networkNode)
{
    m_networkNode = networkNode;
}

QVector<MenuAction> BurnFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;
//    if(type == SpaceArea) {

//    } else if (type == SingleFile){
//        actionKeys << MenuAction::Open;
//        actionKeys << MenuAction::OpenInNewWindow;
//    }else if (type == MultiFiles){

//    }
    return actionKeys;
}


Qt::ItemFlags BurnFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

quint8 BurnFileInfo::supportViewMode() const
{
    return DFileView::IconMode | DFileView::ListMode;
}

QString BurnFileInfo::iconName() const
{
    if (m_networkNode.iconType().startsWith(". GThemedIcon")){
        QString iconType = m_networkNode.iconType();
        QStringList iconNames = iconType.right(iconType.length() - QString(". GThemedIcon").length()).trimmed().split(" ");
        if (iconNames.count() >= 1){
            return iconNames.first();
        }
    }
    return m_networkNode.iconType();
}
