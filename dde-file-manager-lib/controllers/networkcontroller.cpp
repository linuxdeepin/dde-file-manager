/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

const DAbstractFileInfoPointer NetworkController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new NetworkFileInfo(event->url()));
}

const QList<DAbstractFileInfoPointer> NetworkController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    QList<DAbstractFileInfoPointer> infolist;

    foreach (const NetworkNode& node, NetworkManager::NetworkNodes.value(event->url())) {
        NetworkFileInfo* info = new NetworkFileInfo(DUrl(node.url()));
        info->setNetworkNode(node);
        infolist.append(DAbstractFileInfoPointer(info));
    };

    return infolist;
}

