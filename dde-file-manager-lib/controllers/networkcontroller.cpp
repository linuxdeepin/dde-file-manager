/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
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

#include "singleton.h"

class NetworkFileDDirIterator : public DDirIterator
{
public:
    NetworkFileDDirIterator(const DUrl &url, const QObject *event_sender, bool silence)
        : m_url(url)
        , m_sender(event_sender)
        , m_silence(silence)
    {

    }

    DUrl next() override
    {
        m_currentInfo = m_infoList.takeFirst();

        return m_currentInfo->fileUrl();
    }

    bool hasNext() const
    {
        if (initialized) {
            return !m_infoList.isEmpty();
        }

        initialized = true;

        if (!m_silence && NetworkManager::NetworkNodes.value(m_url).isEmpty()) {
            Singleton<NetworkManager>::instance()->fetchNetworks(DFMUrlBaseEvent(m_sender.data(), m_url));
        }

        foreach (const NetworkNode &node, NetworkManager::NetworkNodes.value(m_url)) {
            NetworkFileInfo *info = new NetworkFileInfo(DUrl(node.url()));
            info->setNetworkNode(node);
            m_infoList.append(DAbstractFileInfoPointer(info));
        };

        return !m_infoList.isEmpty();
    }

    void close() override
    {
        if (initialized) {
            NetworkManager::cancelFeatchNetworks();
        }
    }

    QString fileName() const override
    {
        return m_currentInfo->fileName();
    }

    DUrl fileUrl() const override
    {
        return m_currentInfo->fileUrl();
    }

    const DAbstractFileInfoPointer fileInfo() const override
    {
        return m_currentInfo;
    }

    DUrl url() const override
    {
        return m_url;
    }

private:
    mutable bool initialized = false;
    DUrl m_url;
    QPointer<const QObject> m_sender;
    bool m_silence{ false };
    DAbstractFileInfoPointer m_currentInfo;
    mutable QList<DAbstractFileInfoPointer> m_infoList;
};

NetworkController::NetworkController(QObject *parent):
    DAbstractFileController(parent)
{

}

NetworkController::~NetworkController()
{

}

const DAbstractFileInfoPointer NetworkController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    NetworkFileInfo *info = new NetworkFileInfo(event->url());

    for (const NetworkNode &node : NetworkManager::NetworkNodes.value(DUrl(NETWORK_ROOT))) {
        if (DUrl(node.url()) == event->url()) {
            info->setNetworkNode(node);
            break;
        }
    }

    return DAbstractFileInfoPointer(info);
}

const QList<DAbstractFileInfoPointer> NetworkController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    QList<DAbstractFileInfoPointer> infolist;

    if (NetworkManager::NetworkNodes.value(event->url()).isEmpty()) {
        if (event->silent()) {
            // blumia: if silent is enabled, we'll not invoke fetchNetwork here.
            Singleton<NetworkManager>::instance()->fetchNetworks(DFMUrlBaseEvent(this, event->url()));
        }
    }

    foreach (const NetworkNode &node, NetworkManager::NetworkNodes.value(event->url())) {
        NetworkFileInfo *info = new NetworkFileInfo(DUrl(node.url()));
        info->setNetworkNode(node);
        infolist.append(DAbstractFileInfoPointer(info));
    };

    return infolist;
}

const DDirIteratorPointer NetworkController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    bool silence_flag{ event->property(QT_STRINGIFY(DFMGetChildrensEvent::slient)).toBool() };
    return DDirIteratorPointer(new NetworkFileDDirIterator(event->fileUrl(), event->sender().data(), silence_flag));
}

