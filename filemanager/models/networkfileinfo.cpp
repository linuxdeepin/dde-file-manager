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

