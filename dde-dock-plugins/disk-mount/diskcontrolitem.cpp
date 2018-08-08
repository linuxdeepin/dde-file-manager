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

#include "diskcontrolitem.h"

#include "dfileservices.h"
#include "dfmglobal.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QtMath>
#include <QApplication>
#include <QDebug>
#include <QStorageInfo>

#include <dfmdiskmanager.h>
#include <dfmblockdevice.h>
#include <dfmdiskdevice.h>
#include <dfmsettings.h>
#include <DDesktopServices>

DWIDGET_USE_NAMESPACE

DFM_USE_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(DFMSettings, gsGlobal, ("deepin/dde-file-manager", DFMSettings::GenericConfig))

DiskControlItem::DiskControlItem(const DFMBlockDevice *blockDevicePointer, QWidget *parent)
    : QFrame(parent),

      m_unknowIcon(":/icons/resources/unknown.svg"),

      m_diskIcon(new QLabel),
      m_diskName(new QLabel),
      m_diskCapacity(new QLabel),
      m_capacityValueBar(new QProgressBar),
      m_unmountButton(new DImageButton)
{
    mountPoint = blockDevicePointer->mountPoints().first();
    deviceDBusId = blockDevicePointer->path();

    m_diskName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_diskName->setStyleSheet("color:white;");

    m_diskCapacity->setStyleSheet("color:rgba(255, 255, 255, .6);");

    m_capacityValueBar->setTextVisible(false);
    m_capacityValueBar->setFixedHeight(2);
    m_capacityValueBar->setStyleSheet("QProgressBar {"
                                      "border:none;"
                                      "background-color:rgba(255, 255, 255, .1);"
                                      "}"
                                      "QProgressBar::chunk {"
                                      "background-color:rgba(255, 255, 255, .8);"
                                      "}");

    m_unmountButton->setNormalPic(":/icons/resources/unmount-normal.svg");
    m_unmountButton->setHoverPic(":/icons/resources/unmount-hover.svg");
    m_unmountButton->setPressPic(":/icons/resources/unmount-press.svg");
    m_unmountButton->setStyleSheet("margin-top:12px;");

    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(m_diskName);
    infoLayout->addWidget(m_diskCapacity);
    infoLayout->setSpacing(0);
    infoLayout->setContentsMargins(3, 6, 0, 8);

    QHBoxLayout *unmountLayout = new QHBoxLayout;
    unmountLayout->addLayout(infoLayout);
    unmountLayout->addWidget(m_unmountButton);
    unmountLayout->setSpacing(0);
    unmountLayout->setMargin(0);

    QVBoxLayout *progressLayout = new QVBoxLayout;
    progressLayout->addLayout(unmountLayout);
    progressLayout->addWidget(m_capacityValueBar);
    progressLayout->setSpacing(0);
    progressLayout->setContentsMargins(10, 0, 0, 5);

    QHBoxLayout *centralLayout = new QHBoxLayout;
    centralLayout->addWidget(m_diskIcon);
    centralLayout->addLayout(progressLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(0, 0, 5, 0);

    setLayout(centralLayout);
    setObjectName("DiskItem");
    setStyleSheet("QFrame #DiskItem:hover {"
                  "background-color:rgba(255, 255, 255, .1);"
                  "border-radius:4px;"
                  "}");

    connect(m_unmountButton, &DImageButton::clicked, this, [this] { emit requestUnmount(deviceDBusId); });

    QScopedPointer<DFMDiskDevice> diskDev(DFMDiskManager::createDiskDevice(blockDevicePointer->drive()));

    bool isDvd = blockDevicePointer->device().startsWith("/dev/sr");
    bool isRemovable = diskDev->removable();

    if (gsGlobal->value("GenericAttribute", "DisableNonRemovableDeviceUnmount", false).toBool() && isRemovable) {
        m_unmountButton->hide();
    }

    QString iconName = QStringLiteral("drive-harddisk");

    if (isRemovable) {
        iconName = QStringLiteral("drive-removable-media-usb");
    }

    if (isDvd) {
        iconName = QStringLiteral("media-optical");
    }

    QIcon icon = QIcon::fromTheme(iconName, m_unknowIcon);

    qreal devicePixelRatio = qApp->devicePixelRatio();
    QPixmap diskIconPixmap = icon.pixmap(48 * devicePixelRatio , 48 * devicePixelRatio);
    diskIconPixmap.setDevicePixelRatio(devicePixelRatio);
    m_diskIcon->setPixmap(diskIconPixmap);
    m_diskName->setText(QStringLiteral("OwO")); // blumia: correct text should be set in DiskControlItem::showEvent()
    m_capacityValueBar->setMinimum(0);
    m_capacityValueBar->setMaximum(100);
}

DiskControlItem::~DiskControlItem()
{
}

QString DiskControlItem::sizeString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

qreal DiskControlItem::dRound64(qreal num, int count)
{
    if (count <= 0)
        return qRound64(num);

    qreal base = qPow(10, count);

    return qRound64(num * base) / base;
}


const QString DiskControlItem::formatDiskSize(const quint64 num) const
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if ( num >= tb ) {
        total = QString( "%1 TB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / tb), 'f', 1 )) );
    } else if( num >= gb ) {
        total = QString( "%1 GB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / gb), 'f', 1 )) );
    } else if( num >= mb ) {
        total = QString( "%1 MB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / mb), 'f', 1 )) );
    } else if( num >= kb ) {
        total = QString( "%1 KB" ).arg( sizeString(QString::number( dRound64(qreal( num ) / kb),'f',1 )) );
    } else {
        total = QString( "%1 B" ).arg( num );
    }

    return total;
}

void DiskControlItem::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);

    DDesktopServices::showFolder(QUrl::fromLocalFile(mountPoint));
}

void DiskControlItem::showEvent(QShowEvent *e)
{
    QStorageInfo storage_info(mountPoint);
    QScopedPointer<DFMBlockDevice> blDev(DFMDiskManager::createBlockDevice(deviceDBusId));
    bool hasLabelName = true;

    if (blDev->isValid()) {
        QString devName = blDev->idLabel();
        if (devName.isEmpty()) {
            hasLabelName = false;
            devName = QString(tr("%1 Volume")).arg(formatDiskSize(blDev->size()));
        }
        m_diskName->setText(devName);
    }

    if (storage_info.isValid()) {
        qint64 bytesTotal = storage_info.bytesTotal();
        qint64 bytesFree = storage_info.bytesFree();
        if (!hasLabelName) {
            QString devName = QString(tr("%1 Volume")).arg(formatDiskSize(bytesTotal));
            m_diskName->setText(devName);
        }
        m_diskCapacity->setText(QString("%1 / %2")
                                .arg(formatDiskSize(bytesTotal - bytesFree))
                                .arg(formatDiskSize(bytesTotal)));
        m_capacityValueBar->setValue(100 * (bytesTotal - bytesFree) / bytesTotal);
    }

    QFrame::showEvent(e);
}

DFMSettings *getGsGlobal()
{
    return gsGlobal;
}
