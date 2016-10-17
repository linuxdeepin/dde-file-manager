#include "networkcontroller.h"
#include "models/networkfileinfo.h"
#include "gvfs/networkmanager.h"

NetworkController::NetworkController(QObject *parent):
    DAbstractFileController(parent)
{

}

NetworkController::~NetworkController()
{

}

const AbstractFileInfoPointer NetworkController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new NetworkFileInfo(fileUrl));
}

const QList<AbstractFileInfoPointer> NetworkController::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                                    QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                                    bool &accepted) const
{
    Q_UNUSED(filters)
    Q_UNUSED(nameFilters)
    Q_UNUSED(flags)

    QList<AbstractFileInfoPointer> infolist;

    accepted = true;

    foreach (const NetworkNode& node, NetworkManager::NetworkNodes.value(fileUrl)) {
        NetworkFileInfo* info = new NetworkFileInfo(DUrl(node.url()));
        info->setNetworkNode(node);
        infolist.append(AbstractFileInfoPointer(info));
    };

    return infolist;
}

