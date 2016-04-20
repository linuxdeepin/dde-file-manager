#include "udiskdeviceinfo.h"
#include "../filemanager/app/global.h"
#include "../filemanager/shutil/fileutils.h"
#include <QIcon>

UDiskDeviceInfo::UDiskDeviceInfo(UDiskDeviceInfo *info)
    : AbstractFileInfo(DUrl::fromComputerFile("/"))
{
    data->url.setFragment(info->mountPath());
    data->url.setQuery(info->uDiskPath());
    m_size = info->size();
    m_type = info->type();
    m_isMounted = info->isMounted();
    m_mountPath = info->mountPath();
}

UDiskDeviceInfo::UDiskDeviceInfo(const DUrl &url)
    : AbstractFileInfo(url)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(const QString &url)
    : AbstractFileInfo(url)
{

}

UDiskDeviceInfo::UDiskDeviceInfo(const QDBusObjectPath &path)
{
    m_path = path;
    m_blockIface = new QDBusInterface("org.freedesktop.UDisks2",
                                m_path.path(),
                                "org.freedesktop.UDisks2.Block",
                                QDBusConnection::systemBus(),
                                this);

    QDBusObjectPath drive_object = m_blockIface->property("Drive").value<QDBusObjectPath>();

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2",
                                         m_path.path(),
                                         "org.freedesktop.DBus.Properties","PropertiesChanged",
                                         this,
                                         SIGNAL(update()));

    m_driveIface = new QDBusInterface("org.freedesktop.UDisks2",
                                     drive_object.path(),
                                     "org.freedesktop.UDisks2.Drive",
                                     QDBusConnection::systemBus(),
                                     this);
    update();
    if(!m_fileSystem.isEmpty())
        m_fsIface = new QDBusInterface("org.freedesktop.UDisks2",
                                   m_path.path(),
                                   "org.freedesktop.UDisks2.Filesystem",
                                   QDBusConnection::systemBus(),
                                   this);

    QDBusConnection::systemBus().connect(m_blockIface->service(), m_blockIface->path(),
                       "org.freedesktop.DBus.Properties", "PropertiesChanged",
                       this, SLOT(propertiesChanged(QString,QVariantMap,QStringList)));

}

bool UDiskDeviceInfo::mount()
{
    if(m_fsIface == NULL)
        return false;
    QDBusMessage reply = m_fsIface->call("Mount", QVariantMap());
    qDebug() <<  "mount" << reply.arguments().first().toString();
    return false;
}

bool UDiskDeviceInfo::unmount()
{
    if(m_fsIface == NULL)
        return false;
    QDBusMessage reply = m_fsIface->call("Unmount", QVariantMap());
    qDebug() <<  "unmount" << reply.arguments().first().toString();
    return false;
}

bool UDiskDeviceInfo::eject()
{
    return false;
}

bool UDiskDeviceInfo::update()
{
    bool res = false;
    res = setDevFile(m_blockIface->property("Device").toByteArray().data()) || res;

    res = setType(findType()) || res;
    res = setLabel(findLabel()) || res;
    res = setIsExternal(findIsExternal()) || res;
    res = setIconName(findIconName()) || res;

    QStringList mounts = mountPoints();

    res = setIsMounted(mounts.count() != 0) || res;

    res = setIsEjectable(m_driveIface->property("Ejectable").toBool()) || res;
    res = setSize(m_driveIface->property("Size").toULongLong()) || res;
    res = setVendor(m_driveIface->property("Vendor").toString()) || res;
    res = setModel(m_driveIface->property("Model").toString()) || res;
    res = setFileSystem(m_blockIface->property("IdType").toString()) || res;

//    qDebug() << mounts;
//    qDebug() << "media" << m_driveIface->property("Media").toString();
//    qDebug() << "optical" << m_driveIface->property("Optical").toString();
//    qDebug() << "removable" << m_driveIface->property("MediaRemovable").toString();
//    qDebug() << "size" << m_size;
//    qDebug() << "model" << m_model;
//    qDebug() << "vendor" << m_vendor;
//    qDebug() << "filesystem" << m_fileSystem;

    if (!mounts.empty())
        res = setMountPath(mounts.first()) || res;
    return res;
}

QString UDiskDeviceInfo::uDiskPath()
{
    return m_path.path();
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

QString UDiskDeviceInfo::mountPath() const
{
    return m_mountPath;
}

QString UDiskDeviceInfo::iconName()
{
    return m_iconName;
}

qint64 UDiskDeviceInfo::size()
{
    return m_size;
}

QString UDiskDeviceInfo::displayName() const
{
    return FileUtils::formatSize(m_size);
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
    //Todo: Find the type of the device
    //Note that udisk only recognizes cd/dvd rom as removable device
    return Unknown;
}

QString UDiskDeviceInfo::findLabel()
{
    const QString idLabel = m_blockIface->property("IdLabel").toString();
    return idLabel;
}

bool UDiskDeviceInfo::findIsExternal()
{
    return !m_blockIface->property("Removable").toBool();
}

QString UDiskDeviceInfo::findIconName()
{
    return "media device";
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

DUrl UDiskDeviceInfo::parentUrl() const
{
    return DUrl::fromComputerFile("/");
}

void UDiskDeviceInfo::dbusError(const QDBusError &err, const QDBusMessage &msg)
{
    Q_UNUSED(msg);
    qDebug() << "dbus error" << err.message();
    emit error(err.message());
}

void UDiskDeviceInfo::propertiesChanged(const QString &interface, const QVariantMap &changedProp, const QStringList &invalidatedProp)
{
    update();
    emit changed();
}

QVector<AbstractFileInfo::MenuAction> UDiskDeviceInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;
    if(deviceListener->isSystemDisk(mountPath()))
    {
        actionKeys.reserve(6);
        actionKeys << MenuAction::Open << MenuAction::OpenInNewWindow
                   << MenuAction::Separator
                   << MenuAction::Property;
    }
    else
    {
        actionKeys.reserve(6);
        actionKeys << MenuAction::Open << MenuAction::OpenInNewWindow
                   << MenuAction::Separator
                   << MenuAction::Mount << MenuAction::Unmount
                   << MenuAction::Separator
                   << MenuAction::Property;
    }

    return actionKeys;
}

QStringList UDiskDeviceInfo::mountPoints() const
{
    QStringList points;
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.UDisks2",
                                                          m_path.path(),
                                                          "org.freedesktop.DBus.Properties",
                                                          "Get");

    QList<QVariant> args;
    args << "org.freedesktop.UDisks2.Filesystem" << "MountPoints";
    message.setArguments(args);

    QDBusMessage reply = QDBusConnection::systemBus().call(message);

    QList<QByteArray> l;
    foreach (QVariant arg, reply.arguments())
        arg.value<QDBusVariant>().variant().value<QDBusArgument>() >> l;

    foreach (QByteArray p, l)
        points.append(p);

    qDebug() << "mountPoints()" << points;
    return points;
}

