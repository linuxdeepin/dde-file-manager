#include "deviceinfo.h"
#include <QIcon>
#include <QDebug>

DeviceInfo::DeviceInfo(DeviceInfo *deviceInfo)
    : AbstractFileInfo(deviceInfo->getMountPath())
{
    m_mountPath = deviceInfo->getMountPath();
    m_label = deviceInfo->getLabel();
    m_sysPath = deviceInfo->getSysPath();
    m_devLabel = deviceInfo->getDeviceLabel();
    m_uuid = deviceInfo->getUUID();
    m_size = deviceInfo->getSize();
}

DeviceInfo::DeviceInfo(const QString &url)
    : AbstractFileInfo(url)
{

}

DeviceInfo::DeviceInfo(const QString &path,
                       const QString &sysPath,
                       const QString &label,
                       const QString &devLabel,
                       const QString &uuid,
                       const QString &size)
    : AbstractFileInfo(path)
{
    m_mountPath = path;
    m_label = label;
    m_sysPath = sysPath;
    m_devLabel = devLabel;
    m_uuid = uuid;
    m_size = size;
}

QString DeviceInfo::getMountPath()
{
    return m_mountPath;
}

QString DeviceInfo::getLabel()
{
    return m_label;
}

QString DeviceInfo::getSysPath()
{
    return m_sysPath;
}

QString DeviceInfo::getDeviceLabel()
{
    return m_devLabel;
}

QString DeviceInfo::getUUID()
{
    return m_uuid;
}

QString DeviceInfo::getSize()
{
    return m_size;
}

QString DeviceInfo::displayName() const
{
    return formatSize(m_size.toLongLong());
}

bool DeviceInfo::isCanRename() const
{
    return false;
}

bool DeviceInfo::isDir() const
{
    return true;
}

QIcon DeviceInfo::fileIcon() const
{
    return QIcon(":/icons/images/icons/disk_normal_22px.svg");
}

QString DeviceInfo::formatSize( qint64 num ) const
{

    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if ( num >= tb ) total = QString( "%1 TiB" ).arg( QString::number( qreal( num ) / tb, 'f', 3 ) );
    else if ( num >= gb ) total = QString( "%1 GiB" ).arg( QString::number( qreal( num ) / gb, 'f', 2 ) );
    else if ( num >= mb ) total = QString( "%1 MiB" ).arg( QString::number( qreal( num ) / mb, 'f', 1 ) );
    else if ( num >= kb ) total = QString( "%1 KiB" ).arg( QString::number( qreal( num ) / kb,'f',1 ) );
    else total = QString( "%1 bytes" ).arg( num );

    return total;
}
