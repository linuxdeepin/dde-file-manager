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

#include "dfmglobal.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QtMath>
#include <QApplication>
#include <QDebug>
#include <QStorageInfo>

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <dfmsettings.h>
#include <DDesktopServices>

DWIDGET_USE_NAMESPACE

DFM_USE_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(DFMSettings, gsGlobal, ("deepin/dde-file-manager", DFMSettings::GenericConfig))

// it takes the ownership of \a attachedDevicePtr.
DiskControlItem::DiskControlItem(DAttachedDeviceInterface *attachedDevicePtr, QWidget *parent)
    : QFrame(parent),

      m_unknowIcon(":/icons/resources/unknown.svg"),

      m_diskIcon(new QPushButton(this)),
      m_diskName(new QLabel),
      m_diskCapacity(new QLabel),
      m_capacityValueBar(new QProgressBar),
      m_unmountButton(new DImageButton)
{
    attachedDevice.reset(attachedDevicePtr);

    m_diskName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_diskName->setStyleSheet("color:white;");
    m_diskName->setTextFormat(Qt::PlainText);

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

    connect(m_unmountButton, &DImageButton::clicked, this, [this] {
        attachedDevice->detach();
    });

    if (gsGlobal->value("GenericAttribute", "DisableNonRemovableDeviceUnmount", false).toBool() && !attachedDevice->detachable()) {
        m_unmountButton->hide();
    }

    m_diskIcon->setFlat(true);
    m_diskIcon->setIcon(QIcon::fromTheme(attachedDevice->iconName(), m_unknowIcon));
    m_diskIcon->setIconSize(QSize(48, 48));
    m_diskIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_diskIcon->setStyleSheet("padding: 0;");
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

QString DiskControlItem::formatDiskSize(const quint64 num)
{
    QStringList list {" B", " KB", " MB", " GB", " TB"};
    qreal fileSize(num);

    QStringListIterator i(list);
    QString unit = i.next();

    int index = 0;
    while(i.hasNext()) {
        if (fileSize < 1024) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', 1)), unit);
}

void DiskControlItem::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);

    DDesktopServices::showFolder(attachedDevice->mountpointUrl());
}

void DiskControlItem::showEvent(QShowEvent *e)
{
    m_diskName->setText(attachedDevice->displayName());

    if (attachedDevice->deviceUsageValid()) {
        QPair<quint64, quint64> freeAndTotal = attachedDevice->deviceUsage();
        qint64 bytesFree = freeAndTotal.first;
        qint64 bytesTotal = freeAndTotal.second;
        m_diskCapacity->setText(QString("%1 / %2")
                                .arg(formatDiskSize(bytesTotal - bytesFree))
                                .arg(formatDiskSize(bytesTotal)));
        if (bytesTotal > 0) {
            m_capacityValueBar->setValue(100 * (bytesTotal - bytesFree) / bytesTotal);
        }
    }

    QFrame::showEvent(e);
}

DFMSettings *getGsGlobal()
{
    return gsGlobal;
}
