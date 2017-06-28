#include "burncontroller.h"
#include "models/burnfileinfo.h"
#include "gvfs/networkmanager.h"

BurnController::BurnController(QObject *parent):
    DAbstractFileController(parent)
{

}

BurnController::~BurnController()
{

}

const DAbstractFileInfoPointer BurnController::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return DAbstractFileInfoPointer(new BurnFileInfo(fileUrl));
}

const QList<DAbstractFileInfoPointer> BurnController::getChildren(const DUrl &fileUrl, const QStringList &nameFilters,
                                                                    QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                                    bool &accepted) const
{
    Q_UNUSED(filters)
    Q_UNUSED(nameFilters)
    Q_UNUSED(flags)

    QList<DAbstractFileInfoPointer> infolist;

    accepted = true;

    foreach (const NetworkNode& node, NetworkManager::NetworkNodes.value(fileUrl)) {
        BurnFileInfo* info = new BurnFileInfo(DUrl(node.url()));
        info->setNetworkNode(node);
        infolist.append(DAbstractFileInfoPointer(info));
    };

    return infolist;
}

