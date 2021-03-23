/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include "private/dfmvfsmanager_p.h"

#include <unistd.h>

#include <QUrl>
#include <QDebug>
#include <QThread>
#include <QPointer>
#include <QScopedPointer>
#include <QLoggingCategory>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(vfsManager, "vfs.manager")
#else
Q_LOGGING_CATEGORY(vfsManager, "vfs.manager", QtInfoMsg)
#endif

DFM_BEGIN_NAMESPACE

DFMVfsManagerPrivate::DFMVfsManagerPrivate(DFMVfsManager *qq)
    : q_ptr(qq)
{
    m_GVolumeMonitor.reset(g_volume_monitor_get());
    initConnect();
}

DFMVfsManagerPrivate::~DFMVfsManagerPrivate()
{
    Q_Q(DFMVfsManager);

    if (m_handler) {
        delete m_handler;
    }

    g_signal_handlers_disconnect_by_data(m_GVolumeMonitor.data(), q);
}

void DFMVfsManagerPrivate::GVolumeMonitorMountAddedCb(GVolumeMonitor *, GMount *mount, DFMVfsManager* managerPointer)
{
    DFMGFile rootFile(g_mount_get_root(mount));
    DFMGCChar rootUriCStr(g_file_get_uri(rootFile.data()));
    QString rootUrlStr(rootUriCStr.data());
    QUrl url(rootUrlStr);
    if (url.scheme() == "file") return;
//    if (g_mount_is_shadowed(mount)) return; // is_shadowed

    QUrl deviceUrl;
    deviceUrl.setScheme("device");
    deviceUrl.setPath(rootUrlStr);

    emit managerPointer->vfsAttached(deviceUrl);
    emit managerPointer->vfsDeviceListInfoChanged();
}

void DFMVfsManagerPrivate::GVolumeMonitorMountRemovedCb(GVolumeMonitor *, GMount *mount, DFMVfsManager* managerPointer)
{
    DFMGFile rootFile(g_mount_get_root(mount));
    DFMGCChar rootUriCStr(g_file_get_uri(rootFile.data()));
    QString rootUrlStr(rootUriCStr.data());
    QUrl url(rootUrlStr);
    if (url.scheme() == "file") return;
//    if (g_mount_is_shadowed(mount)) return; // is_shadowed

    QUrl deviceUrl;
    deviceUrl.setScheme("device");
    deviceUrl.setPath(rootUrlStr);

    emit managerPointer->vfsDetached(deviceUrl);
    emit managerPointer->vfsDeviceListInfoChanged();
}

void DFMVfsManagerPrivate::GVolumeMonitorMountChangedCb(GVolumeMonitor *, GMount *mount, DFMVfsManager* managerPointer)
{
    Q_UNUSED(managerPointer);

    DFMGFile rootFile(g_mount_get_root(mount));
    DFMGCChar rootUriCStr(g_file_get_uri(rootFile.data()));
    QString rootUrlStr(rootUriCStr.data());
    QUrl url(rootUrlStr);
    if (url.scheme() == "file") return;

    emit managerPointer->vfsDeviceListInfoChanged();
}

void DFMVfsManagerPrivate::initConnect()
{
    Q_Q(DFMVfsManager);

    // check if is not root.
    if (getuid() != 0) {
        g_signal_connect(m_GVolumeMonitor.data(), "mount-added", (GCallback)&DFMVfsManagerPrivate::GVolumeMonitorMountAddedCb, q);
        g_signal_connect(m_GVolumeMonitor.data(), "mount-removed", (GCallback)&DFMVfsManagerPrivate::GVolumeMonitorMountRemovedCb, q);
        g_signal_connect(m_GVolumeMonitor.data(), "mount-changed", (GCallback)&DFMVfsManagerPrivate::GVolumeMonitorMountChangedCb, q);
    }
}

/*!
 * \brief Get a list of all attached virtual filesystem.
 *
 * Only virtual filesystems (i.e. remote/network locations) will be returned.
 *
 * \return QList of attached virtual filesystem with their root url.
 */
const QList<QUrl> DFMVfsManager::getVfsList()
{
    Q_D(DFMVfsManager);

    QSet<QUrl> result;

    DFMGMountList mountList(g_volume_monitor_get_mounts(d->m_GVolumeMonitor.data()));
    GMount* mount = nullptr;
    GList* iter;
    for (iter = mountList.data(); iter != nullptr; iter = iter->next) {
        mount = (GMount*)iter->data;
        DFMGFile rootFile(g_mount_get_root(mount));
        DFMGCChar rootUriCStr(g_file_get_uri(rootFile.data()));
        QString rootUrlStr(rootUriCStr.data());
        QUrl urlForCheck(rootUrlStr);
        if (urlForCheck.scheme() == "file") continue;
        if (g_mount_is_shadowed(mount)) continue;
        QUrl url;
        url.setScheme("device");
        url.setPath(rootUrlStr);
        result << url;
    }

    return result.toList();
}

/*! \class DFMVfsManager

    \brief DFMVfsManager manage all virtual filesystem.

    Virtual filesystem here means it's not a physical device in local computer, i.e. it's a remote/network
    device. DFMVfsManager manage the state of already mounted devices and provide signal when a new vfs is
    mounted(attached) to the computer or a vfs mount point get unmounted(detached).

    We use the word attach and detach rather than mount and unmount because one *network* location will
    always comes with one available mount point, so a drive / partition / volume mount / unmount event will
    always consider as a same event.

    We use the URI which is used to mount the device as a identifier, use DFMVfsManager::getVfsList() to
    get a list of already attached vfs list, and use DFMVfsDevice to manage them when needed.

    \sa DFMVfsDevice, DDiskManager
 */

DFMVfsManager::DFMVfsManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMVfsManagerPrivate(this))
{

}

DFMVfsManager::~DFMVfsManager()
{

}


bool DFMVfsManager::attach(const QUrl &url)
{
    if (!url.isValid() || url.scheme() == "file" || url.scheme().isEmpty()) {
        return false;
    }

    QScopedPointer<DFMVfsDevice> dev(DFMVfsDevice::createUnsafe(url, nullptr));

    if (eventHandler()) {
        dev->setEventHandler(eventHandler());
    }

    return dev->attach();
}

DFMVfsAbstractEventHandler *DFMVfsManager::eventHandler() const
{
    Q_D(const DFMVfsManager);

    return d->m_handler;
}

void DFMVfsManager::setEventHandler(DFMVfsAbstractEventHandler *handler, QThread *threadOfHandler)
{
    Q_D(DFMVfsManager);

    d->m_handler = handler;
    d->m_threadOfEventHandler = threadOfHandler;
}


DFM_END_NAMESPACE
