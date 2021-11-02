/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "durl.h"

#include "singleton.h"

#include <QMutex>

Q_GLOBAL_STATIC(QMutex, fetchNetworksMutex)

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

    bool hasNext() const override
    {
        if (initialized) {
            return !m_infoList.isEmpty();
        }

        initialized = true;

        fetchNetworksMutex->lock();
        if (!m_silence && NetworkManager::NetworkNodes.value(m_url).isEmpty()) {
            Singleton<NetworkManager>::instance()->fetchNetworks(DFMUrlBaseEvent(m_sender.data(), m_url));
        }
        fetchNetworksMutex->unlock();

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
        if (m_currentInfo) {
            return m_currentInfo->fileName();
        }
        else return "";
    }

    DUrl fileUrl() const override
    {
        if (m_currentInfo) {
            return m_currentInfo->fileUrl();
        }
        else return DUrl("");
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
    auto nodes = NetworkManager::NetworkNodes.value(DUrl(NETWORK_ROOT));
    auto iter = std::find_if(nodes.begin(), nodes.end(), [event](const NetworkNode & node) {
        return DUrl(node.url()) == event->url();
    });

    if (iter != nodes.end()) {
        info->setNetworkNode(*iter);
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

DUrlList NetworkController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    Q_UNUSED(event)

    return DUrlList(); // disabled for now.
}

