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

#include "diskcontrolitem.h"
#include "qdiskinfo.h"

#include "dfileservices.h"
#include "gvfsmountmanager.h"
#include "dfmglobal.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QtMath>
#include <QApplication>
#include <QDebug>

DWIDGET_USE_NAMESPACE

DiskControlItem::DiskControlItem(const QDiskInfo &info, QWidget *parent)
    : QFrame(parent),

      m_unknowIcon(":/icons/resources/unknown.svg"),

      m_diskIcon(new QLabel),
      m_diskName(new QLabel),
      m_diskCapacity(new QLabel),
      m_capacityValueBar(new QProgressBar),
      m_unmountButton(new DImageButton)
{
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

    connect(m_unmountButton, &DImageButton::clicked, this, [this] { emit requestUnmount(m_info.id()); });

    if (DFMGlobal::isDisableUnmount(info)){
        m_unmountButton->hide();
    }

    updateInfo(info);
}

void DiskControlItem::updateInfo(const QDiskInfo &info)
{
    m_info = info;
    qreal devicePixelRatio = qApp->devicePixelRatio();
    QPixmap diskIconPixmap = QIcon::fromTheme(info.iconName(), m_unknowIcon).pixmap(48 * devicePixelRatio , 48 * devicePixelRatio);
    diskIconPixmap.setDevicePixelRatio(devicePixelRatio);
    m_diskIcon->setPixmap(diskIconPixmap);
    if (!info.name().isEmpty())
        m_diskName->setText(info.name());
    else
        m_diskName->setText(tr("Unknown device"));
    if (info.total())
        m_diskCapacity->setText(QString("%1/%2").arg(formatDiskSize(info.used())).arg(formatDiskSize(info.total())));
    else if (info.name().isEmpty())
        m_diskCapacity->clear();
    else
        m_diskCapacity->setText(tr("Unknown volume"));
    m_capacityValueBar->setMinimum(0);
    m_capacityValueBar->setMaximum(100);
    if (info.total() > 0)
        m_capacityValueBar->setValue(100 * info.used() / info.total());
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

//    emit requestOpenDrive(m_info.id());
    DFileService::instance()->openFile(this, GvfsMountManager::getRealMountUrl(m_info));
}
