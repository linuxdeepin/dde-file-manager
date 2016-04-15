#include "udiskdeviceinfo.h"
#include <QIcon>

UDiskDeviceInfo::UDiskDeviceInfo(const QDBusObjectPath &path)
{
    m_dbus = new QDBusInterface("org.freedesktop.UDisks",
                                path.path(),
                                "org.freedesktop.UDisks.Device",
                                QDBusConnection::systemBus(),
                                this);
}

bool UDiskDeviceInfo::mount()
{

}

bool UDiskDeviceInfo::unmount()
{

}

bool UDiskDeviceInfo::eject()
{

}

QString UDiskDeviceInfo::devFile()
{
    return m_devFile;
}

QString UDiskDeviceInfo::label()
{
    return m_label;
}

QString UDiskDeviceInfo::vendor()
{
    return m_vendor;
}

QString UDiskDeviceInfo::model()
{
    return m_model;
}

QString UDiskDeviceInfo::fileSystem()
{
    return m_fileSystem;
}

QString UDiskDeviceInfo::mountPath()
{
    return m_mountPath;
}

QString UDiskDeviceInfo::iconName()
{
    return m_iconName;
}

void UDiskDeviceInfo::setDevFile(const QString &text)
{
    m_devFile = text;
}

void UDiskDeviceInfo::setLabel(const QString &text)
{
    m_label = text;
}

void UDiskDeviceInfo::setVendor(const QString &text)
{
    m_vendor = text;
}

void UDiskDeviceInfo::setModel(const QString &text)
{
    m_model = text;
}

void UDiskDeviceInfo::setFileSystem(const QString &text)
{
    m_fileSystem = text;
}

void UDiskDeviceInfo::setMountPath(const QString &text)
{
    m_mountPath = text;
}

void UDiskDeviceInfo::setIconName(const QString &text)
{
    m_iconName = text;
}

qint64 UDiskDeviceInfo::size()
{
    return m_size;
}

UDiskDeviceInfo::Type UDiskDeviceInfo::type()
{
    return m_type;
}

bool UDiskDeviceInfo::isValid()
{
    return m_isValid;
}

bool UDiskDeviceInfo::isExternal()
{
    return m_isExternal;
}

bool UDiskDeviceInfo::isMounted()
{
    return m_isMounted;
}

bool UDiskDeviceInfo::isEjectable()
{
    return m_isEjectable;
}

UDiskDeviceInfo::Type UDiskDeviceInfo::findType()
{
    if (m_dbus->property("DeviceIsOpticalDisc").toBool())
            return Optical;

    const QString compatibility = m_dbus->property("DriveMediaCompatibility").toString();
    const QString idUsage = m_dbus->property("IdUsage").toString();

    if (m_dbus->property("DeviceIsDrive").toBool())
    {
        if (compatibility == "floppy")
            return Fdd;

        if (idUsage == "filesystem")
            return Drive;

        return Unknown;
    }

    if (m_dbus->property("DeviceIsPartition").toBool())
    {
        if (idUsage == "filesystem")
            return Partition;

        return Unknown;
    }

    return Unknown;
}

QString UDiskDeviceInfo::findLabel()
{
    const QString idLabel = m_dbus->property("IdLabel").toString();

    if (m_type ==  Fdd)
        return tr("Floppy drive");

    if (m_type ==  Optical)
        return idLabel;


    const QString driveVendor = m_dbus->property("DriveVendor").toString();
    const QString driveModel  = m_dbus->property("DriveModel").toString();
    const qulonglong size = m_dbus->property("DeviceSize").toULongLong();

    QString label;
    if (!idLabel.isEmpty())
    {
        label = idLabel;
    }
    else
    {
       if (!driveVendor.isEmpty())
            label = driveVendor;

       if (!driveModel.isEmpty())
            label += QString(" - %1").arg(driveModel);
    }

    if (label.isEmpty())
        label = m_devFile;

    if (m_size)
        label += QString(" [%3]").arg(formatSize(size));

    return label;
}

bool UDiskDeviceInfo::findIsExternal()
{
    return !m_dbus->property("DeviceIsSystemInternal").toBool();
}

QString UDiskDeviceInfo::findIconName()
{
    const QString media = m_dbus->property( "DriveMedia" ).toString();

    switch (m_type)
    {
    case Drive:
    case Partition:
        {
            // handle drives
            const QString conn = m_dbus->property( "DriveConnectionInterface" ).toString();

            if (conn == "usb")
                return "drive-removable-media-usb";

            return "drive-removable-media";
        }
    case Fdd:
        {
            return "media-floppy";
        }
    case Optical:
        {
            bool isWritable = m_dbus->property( "OpticalDiscIsBlank" ).toBool() ||
                              m_dbus->property("OpticalDiscIsAppendable").toBool();

            if (isWritable)
                return "media-optical-recordable";

            if (media.startsWith("optical_dvd") ||
                media.startsWith( "optical_hddvd"))
                return "media-optical-dvd";

            if (media.startsWith("optical_bd"))
                return "media-optical-blu-ray";

            return "media-optical";
        }
    case Unknown:
        {
            return "drive-harddisk";
        }
    }

    return "drive-harddisk";
}

QStringList UDiskDeviceInfo::mountPoints() const
{

}

bool UDiskDeviceInfo::isCanRename() const
{
    return false;
}

QIcon UDiskDeviceInfo::fileIcon() const
{
    return QIcon(":/icons/images/icons/disk_normal_22px.svg");
}

bool UDiskDeviceInfo::isDir() const
{
    return true;
}

QString UDiskDeviceInfo::formatSize( qint64 num ) const
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

