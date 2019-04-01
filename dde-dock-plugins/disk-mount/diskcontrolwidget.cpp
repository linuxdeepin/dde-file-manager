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

#include "diskcontrolwidget.h"
#include "diskcontrolitem.h"
#include "dattachedudisks2device.h"
#include "dattachedvfsdevice.h"

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <dfmvfsdevice.h>
#include <dfmsettings.h>
#include <dfmvfsmanager.h>
#include <DDesktopServices>

#include <QDebug>
#include <QProcess>
#include <QThreadPool>
#include <QtConcurrent>
#include <QScrollBar>
#include <QDebug>
#include <DDBusSender>

#define WIDTH           300

DWIDGET_USE_NAMESPACE

DFM_USE_NAMESPACE

DiskControlWidget::DiskControlWidget(QWidget *parent)
    : QScrollArea(parent),
      m_centralLayout(new QVBoxLayout),
      m_centralWidget(new QWidget)
{
    m_centralWidget->setLayout(m_centralLayout);
    m_centralWidget->setFixedWidth(WIDTH);

    m_vfsManager = new DFMVfsManager;

    setWidget(m_centralWidget);
    setFixedWidth(WIDTH);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setSingleStep(7);
    setStyleSheet("background-color:transparent;");
    m_diskManager = new DDiskManager(this);
    initConnect();
}

DiskControlWidget::~DiskControlWidget()
{
    delete m_vfsManager;
}

void DiskControlWidget::initConnect()
{
    connect(m_diskManager, &DDiskManager::diskDeviceAdded, this, [this](const QString &path) {
        // blumia: Workaround. Wait for udisks2 add new device to device list.
        QTimer::singleShot(500, this, [=](){
            onDriveConnected(path);
        });
    });
    connect(m_diskManager, &DDiskManager::diskDeviceRemoved, this, &DiskControlWidget::onDriveDisconnected);
    connect(m_diskManager, &DDiskManager::mountAdded, this, &DiskControlWidget::onMountAdded);
    connect(m_diskManager, &DDiskManager::mountRemoved, this, &DiskControlWidget::onMountRemoved);
    connect(m_diskManager, &DDiskManager::fileSystemAdded, this, &DiskControlWidget::onVolumeAdded);
    connect(m_diskManager, &DDiskManager::fileSystemRemoved, this, &DiskControlWidget::onVolumeRemoved);

    connect(m_vfsManager, &DFMVfsManager::vfsDeviceListInfoChanged, this, &DiskControlWidget::onDiskListChanged);
}

void DiskControlWidget::startMonitor()
{
    m_diskManager->setWatchChanges(true);
    onDiskListChanged();
}

/*
 *
 * TODO: move this thing into dtkcore or somewhere...
 *       There is also a FileUtils::getKernelParameters() in dde-file-manager.
 * blumia: for writing unit test, try validate the result with `tr ' ' '\n' < /proc/cmdline`
 */
QMap<QString, QString> getKernelParameters()
{
    QFile cmdline("/proc/cmdline");
    cmdline.open(QIODevice::ReadOnly);
    QByteArray content = cmdline.readAll();

    QByteArrayList paraList(content.split(' '));

    QMap<QString, QString> result;
    result.insert("_ori_proc_cmdline", content);

    for (const QByteArray& onePara : paraList) {
        int equalsIdx = onePara.indexOf('=');
        QString key = equalsIdx == -1 ? onePara.trimmed() : onePara.left(equalsIdx).trimmed();
        QString value = equalsIdx == -1 ? QString() : onePara.right(equalsIdx).trimmed();
        result.insert(key, value);
    }

    return result;
}

void DiskControlWidget::doStartupAutoMount()
{
    // check if we are in live system, don't do auto mount if we are in live system.
    static QMap<QString, QString> cmdline = getKernelParameters();
    if (cmdline.value("boot", "") == QStringLiteral("live")) {
        autoMountDisabled = true;
        return;
    }

    if (getGsGlobal()->value("GenericAttribute", "AutoMount", false).toBool() == false) {
        return;
    }

    QStringList blDevList = m_diskManager->blockDevices();
    for (const QString& blDevStr : blDevList) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));

        if (blDev->isEncrypted()) continue;
        if (blDev->hintIgnore()) continue;

        if (blDev->hasFileSystem() && blDev->mountPoints().isEmpty()) {
            blDev->mount({{"auth.no_user_interaction", true}});
        }
    }
}

void DiskControlWidget::unmountAll()
{
    QStringList blockDevices = m_diskManager->blockDevices();

    for (const QString & blDevStr : blockDevices) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));
        if (blDev->hasFileSystem() /* && DFMSetting*/ && !blDev->mountPoints().isEmpty() && !blDev->hintIgnore()) {
            QByteArray mountPoint = blDev->mountPoints().first();
            if (mountPoint != QStringLiteral("/boot") && mountPoint != QStringLiteral("/") && mountPoint != QStringLiteral("/home")) {
                QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blDev->drive()));
                blDev->unmount({});
                if (diskDev->removable()) {
                    diskDev->eject({});
                }
            }
        }
    }

    QList<QUrl> vfsDevices = m_vfsManager->getVfsList();
    for (const QUrl & vfsDevUrl : vfsDevices) {
        DFMVfsDevice* vfsDev = DFMVfsDevice::create(vfsDevUrl);
        if (vfsDev) {
            vfsDev->detachAsync();
        }
    }
}

void DiskControlWidget::onDiskListChanged()
{
    while (QLayoutItem *item = m_centralLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    int mountedCount = 0;

    QStringList blDevList = m_diskManager->blockDevices();
    for (const QString& blDevStr : blDevList) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));
        if (blDev->hasFileSystem() && !blDev->mountPoints().isEmpty() && !blDev->hintIgnore() && !blDev->isLoopDevice()) {
            QByteArray mountPoint = blDev->mountPoints().first();
            if (mountPoint != QStringLiteral("/boot") && mountPoint != QStringLiteral("/") && mountPoint != QStringLiteral("/home")) {
                mountedCount++;
                DAttachedUdisks2Device *dad = new DAttachedUdisks2Device(blDev.data());
                DiskControlItem *item = new DiskControlItem(dad, this);
                m_centralLayout->addWidget(item);
            }
        }
    }

    QList<QUrl> urlList = m_vfsManager->getVfsList();
    for (const QUrl& oneUrl : urlList) {
        DAttachedVfsDevice *dad = new DAttachedVfsDevice(oneUrl);
        if (dad->isValid()) {
            mountedCount++;
            DiskControlItem *item = new DiskControlItem(dad, this);
            m_centralLayout->addWidget(item);
        } else {
            delete dad;
        }
    }

    emit diskCountChanged(mountedCount);

    const int contentHeight = mountedCount * 70;
    const int maxHeight = std::min(contentHeight, 70 * 6);

    m_centralWidget->setFixedHeight(contentHeight);
    setFixedHeight(maxHeight);

    verticalScrollBar()->setPageStep(maxHeight);
    verticalScrollBar()->setMaximum(contentHeight - maxHeight);
}

void DiskControlWidget::onDriveConnected(const QString &deviceId)
{
    QScopedPointer<DDiskDevice> diskDevice(DDiskManager::createDiskDevice(deviceId));
    if (diskDevice->removable()) {
        DDesktopServices::playSystemSoundEffect("device-added");

        if (autoMountDisabled) {
            return;
        }

        bool mountAndOpen = false;

        // Check if we need do auto mount..
        getGsGlobal()->reload();
        if (getGsGlobal()->value("GenericAttribute", "AutoMountAndOpen", false).toBool()) {
            // mount and open
            mountAndOpen = true;
        } else if (getGsGlobal()->value("GenericAttribute", "AutoMount", false).toBool()) {
            // mount
            // no flag there..
        } else {
            // no need to do auto mount, return.
            return;
        }

        // Do auto mount stuff..
        QStringList blDevList = m_diskManager->blockDevices();
        for (const QString& blDevStr : blDevList) {
            QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blDevStr));

            if (blDev->drive() != deviceId) continue;
            if (blDev->isEncrypted()) continue;
            if (blDev->hintIgnore()) continue;

            if (blDev->hasFileSystem() && blDev->mountPoints().isEmpty()) {

                // blumia: if mount&open enabled and dde-file-manager also got installed, use dde-file-manager.
                //         using mount scheme with udisks sub-scheme to give user a *device is mounting* feedback.
                if (mountAndOpen && !QStandardPaths::findExecutable(QStringLiteral("dde-file-manager")).isEmpty()) {
                    QString mountUrlStr = "mount://fromMountPlugin#udisks://" + blDevStr;
                    QProcess::startDetached(QStringLiteral("dde-file-manager"), {mountUrlStr});
                    return;
                }

                QString mountPoint = blDev->mount({});
                if (mountAndOpen && !mountPoint.isEmpty()) {
                    DDesktopServices::showFolder(QUrl::fromLocalFile(mountPoint));
                }
            }
        }
    }
}

void DiskControlWidget::onDriveDisconnected()
{
    DDesktopServices::playSystemSoundEffect("device-removed");
    DDBusSender()
        .service("org.freedesktop.Notifications")
        .path("/org/freedesktop/Notifications")
        .interface("org.freedesktop.Notifications")
        .method(QString("Notify"))
        .arg(qApp->applicationName())
        .arg(static_cast<uint>(0))
        .arg(QString("media-eject"))
        .arg(QObject::tr("Device has been removed"))
        .arg(QString())
        .arg(QStringList())
        .arg(QVariantMap())
        .arg(5000).call();
    onDiskListChanged();
}

void DiskControlWidget::onMountAdded()
{
    onDiskListChanged();
}

void DiskControlWidget::onMountRemoved(const QString &blockDevicePath, const QByteArray &mountPoint)
{
    Q_UNUSED(mountPoint);
    // if it's a removable device, don't emit list changed signal.
    // when eject done, it will got emited from onDriveDisconnected.
    QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(blockDevicePath));
    if (blDev) {
        QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blDev->drive()));
        if (diskDev && diskDev->removable()) {
            return;
        }
    }

    onDiskListChanged();
}

void DiskControlWidget::onVolumeAdded()
{
    onDiskListChanged();
}

void DiskControlWidget::onVolumeRemoved()
{
    onDiskListChanged();
}

void DiskControlWidget::unmountDisk(const QString &diskId) const
{
    QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(diskId));
    QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(blDev->drive()));
    blDev->unmount({});
    if (diskDev->optical()) { // is optical
        if (diskDev->ejectable()) {
            diskDev->eject({});
        }
    }
}
