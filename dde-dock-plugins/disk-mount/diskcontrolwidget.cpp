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
#include "gvfsmountmanager.h"
#include "qdrive.h"
#include "dfmsetting.h"
#include "dfmglobal.h"
#include <QDebug>
#include <QProcess>
#include <QThreadPool>
#include <QtConcurrent>
#include <QDebug>

#define WIDTH           300

DiskControlWidget::DiskControlWidget(QWidget *parent)
    : QScrollArea(parent),

      m_centralLayout(new QVBoxLayout),
      m_centralWidget(new QWidget)
{
    m_centralWidget->setLayout(m_centralLayout);
    m_centralWidget->setFixedWidth(WIDTH);

    setWidget(m_centralWidget);
    setFixedWidth(WIDTH);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color:transparent;");
    m_dfmsettings = DFMSetting::instance();
    m_gvfsMountManager = GvfsMountManager::instance();
    m_gvfsMountManager->setAutoMountSwitch(true);
    initConnect();
}

void DiskControlWidget::initConnect()
{
    connect(m_gvfsMountManager, &GvfsMountManager::loadDiskInfoFinished, this, &DiskControlWidget::onDiskListChanged);
    connect(m_gvfsMountManager, &GvfsMountManager::drive_connected, this,  &DiskControlWidget::onDrive_connected);
    connect(m_gvfsMountManager, &GvfsMountManager::drive_disconnected, this,  &DiskControlWidget::onDrive_disconnected);
    connect(m_gvfsMountManager, &GvfsMountManager::mount_added, this,  &DiskControlWidget::onMount_added);
    connect(m_gvfsMountManager, &GvfsMountManager::mount_removed, this, &DiskControlWidget::onMount_removed);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_added, this, &DiskControlWidget::onVolume_added);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_removed, this, &DiskControlWidget::onVolume_removed);
    connect(m_gvfsMountManager, &GvfsMountManager::volume_changed, this, &DiskControlWidget::onVolume_changed);
}

void DiskControlWidget::startMonitor()
{

    QtConcurrent::run(QThreadPool::globalInstance(), m_gvfsMountManager,
                                             &GvfsMountManager::startMonitor);
}

void DiskControlWidget::unmountAll()
{
    foreach (const QDiskInfo& info, GvfsMountManager::DiskInfos) {
        qDebug() << "unmount " << info.id() << "DFMGlobal::isDisableUnmount" << DFMGlobal::isDisableUnmount(info);
        if (DFMGlobal::isDisableUnmount(info)){
            qDebug() << "disable unmount native disk" << info;
            continue;
        }
        unmountDisk(info.id());
    }
}

void DiskControlWidget::onDiskListChanged()
{
    qDebug() << "===============" << GvfsMountManager::DiskInfos;
    while (QLayoutItem *item = m_centralLayout->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    int mountedCount = 0;
    for (auto info : GvfsMountManager::DiskInfos)
    {
        if (info.mounted_root_uri().isEmpty())
            continue;
        else
            ++mountedCount;

        DiskControlItem *item = new DiskControlItem(info, this);

        connect(item, &DiskControlItem::requestUnmount, this, &DiskControlWidget::unmountDisk);

        m_centralLayout->addWidget(item);
    }
    emit diskCountChanged(mountedCount);

    const int contentHeight = mountedCount * 70;
    const int maxHeight = std::min(contentHeight, 70 * 6);

    m_centralWidget->setFixedHeight(contentHeight);
    setFixedHeight(maxHeight);
}

void DiskControlWidget::onDrive_connected(const QDrive &drive)
{
    qDebug() << drive;
    if (drive.is_removable())
        DFMGlobal::playSound(QUrl::fromLocalFile("/usr/share/sounds/deepin/stereo/device-added.ogg"));
}

void DiskControlWidget::onDrive_disconnected(const QDrive &drive)
{
    qDebug() << drive;
    if (drive.is_removable())
        DFMGlobal::playSound(QUrl::fromLocalFile("/usr/share/sounds/deepin/stereo/device-removed.ogg"));
}

void DiskControlWidget::onMount_added(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::onMount_removed(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::onVolume_added(const QDiskInfo &diskInfo)
{
    onDiskListChanged();

    if (GvfsMountManager::isDeviceCrypto_LUKS(diskInfo))
        return;

    GvfsMountManager* gvfsMountManager = GvfsMountManager::instance();
    DFMSetting* globalSetting = DFMSetting::instance();
    qDebug() << "AutoMountSwitch:" << m_gvfsMountManager->getAutoMountSwitch();
    qDebug() << "isAutoMount:" << globalSetting->isAutoMount();
    qDebug() << "isAutoMountAndOpen:" << globalSetting->isAutoMountAndOpen();
    if (m_gvfsMountManager->getAutoMountSwitch()){
        if (diskInfo.is_removable()) {
            if (globalSetting->isAutoMountAndOpen()) {
                gvfsMountManager->mount(diskInfo);
                QProcess::startDetached("dde-file-manager", {"computer:///"});
            } else if (globalSetting->isAutoMount()) {
                gvfsMountManager->mount(diskInfo);
            }
        } else if (globalSetting->isAutoMount()) {
            gvfsMountManager->mount(diskInfo);
        }
    }
}

void DiskControlWidget::onVolume_removed(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::onVolume_changed(const QDiskInfo &diskInfo)
{
    Q_UNUSED(diskInfo)
    onDiskListChanged();
}

void DiskControlWidget::unmountDisk(const QString &diskId) const
{
    m_gvfsMountManager->unmount(diskId);
}
