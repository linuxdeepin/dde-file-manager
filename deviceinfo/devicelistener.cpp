#include "devicelistener.h"
#include "deviceinfo.h"
#include "../filemanager/app/global.h"
#include "../filemanager/app/filesignalmanager.h"
#include "../filemanager/controllers/fileservices.h"

DeviceListener::DeviceListener()
{
    m_udev = udev_new();
    m_udevMonitor = udev_monitor_new_from_netlink(m_udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(m_udevMonitor, "block", 0);
    udev_monitor_enable_receiving(m_udevMonitor);
    int fd = udev_monitor_get_fd(m_udevMonitor);

    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &DeviceListener::deviceReceived);

    fd = open("/proc/self/mounts", O_RDONLY, 0);
    QSocketNotifier *mountedNotifier = new QSocketNotifier(fd, QSocketNotifier::Write);
    connect(mountedNotifier, &QSocketNotifier::activated,
            fileSignalManager, &FileSignalManager::deviceMounted, Qt::QueuedConnection);

    fileService->setFileUrlHandler(COMPUTER_SCHEME, "", this);
}

DeviceListener::~DeviceListener()
{
    udev_monitor_unref(m_udevMonitor);
    udev_unref(m_udev);
}

QList<DeviceInfo *> DeviceListener::loadDeivces()
{
    udev_enumerate *enume = udev_enumerate_new(m_udev);
    udev_enumerate_add_match_subsystem(enume, "block");
    udev_enumerate_scan_devices(enume);
    udev_list_entry *devices;
    devices = udev_enumerate_get_list_entry(enume);
    udev_list_entry *e;
    udev_list_entry_foreach(e, devices) {
        const char *path = udev_list_entry_get_name(e);

        udev_device *dev = udev_device_new_from_syspath(m_udev, path);

        addDevice(dev);

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enume);

    return m_deviceInfos;
}

bool DeviceListener::isTargetDevice(udev_device *dev)
{
    // const char *dev_path = udev_device_get_sysname(dev);
    const char *is_removable = udev_device_get_sysattr_value(dev, "removable");
    const char *is_partition = udev_device_get_sysattr_value(dev, "partition");

    udev_device *par = udev_device_get_parent(dev);
    const char *par_is_removable = udev_device_get_sysattr_value(par, "removable");

    Q_UNUSED(is_removable)
    Q_UNUSED(par_is_removable)

    // qDebug() << path << dev_path << is_removable << is_partition << par_is_removable;

    if (is_partition) { // && ((is_removable && *is_removable == '1') || (par_is_removable && *par_is_removable == 1))) {
        return true;
    } else {
        return false;
    }
}

QString DeviceListener::deviceLabel(udev_device *dev)
{
    udev_list_entry *l = udev_device_get_devlinks_list_entry(dev);
    udev_list_entry *e;
    udev_list_entry_foreach(e, l)
    {
        QString devlink = udev_list_entry_get_name(e);
        if (devlink.startsWith("/dev/disk/by-label"))
        {
            return devlink.split("/").last();
        }
    }
    QString devpath = udev_device_get_devpath(dev);
    return devpath.split("/").last();
}

QString DeviceListener::deviceUUID(udev_device *dev)
{
    udev_list_entry *l = udev_device_get_devlinks_list_entry(dev);
    udev_list_entry *e;
    udev_list_entry_foreach(e, l)
    {
        QString devlink = udev_list_entry_get_name(e);
        if (devlink.startsWith("/dev/disk/by-uuid"))
        {
            return devlink.split("/").last();
        }
    }
    QString devpath = udev_device_get_devpath(dev);
    return devpath.split("/").last();
}

DeviceInfo * DeviceListener::addDevice(udev_device *dev)
{
    if (!isTargetDevice(dev)) {
        return NULL;
    }
    const char *path = udev_device_get_syspath(dev);
    if(m_deviceMap.contains(path))
        return NULL;
    QString mountPath = mountPoint(dev);
    QStorageInfo info(mountPath);
    QString label = info.displayName();
    QString uuid = deviceUUID(dev);
    QString size = udev_device_get_sysattr_value(dev, "size");
    if(size == "0") // Todo: may not be reliable on zero size
        return NULL;
    DeviceInfo * device = new DeviceInfo(mountPath, path, label, deviceLabel(dev), uuid, size);
    m_deviceInfos.append(device);
    m_deviceMap.insert(path, device);
    return device;
}

DeviceInfo * DeviceListener::removeDevice(udev_device *dev)
{
    const char *path = udev_device_get_syspath(dev);
    for(int i = 0; i < m_deviceInfos.size(); i++)
    {
        DeviceInfo * info = m_deviceInfos.at(i);
        if(info->getSysPath() == path)
        {
            m_deviceInfos.removeOne(info);
            m_deviceMap.remove(path);
            return info;
        }
    }
    return NULL;
}

void DeviceListener::deviceReceived(int)
{
    udev_device *dev = udev_monitor_receive_device(m_udevMonitor);

    const char *action = udev_device_get_action(dev);

    if (strcmp(action, "add") == 0)
    {
        DeviceInfo * localInfo = addDevice(dev);
        if(localInfo)
        {
            qDebug() << "device added";
            emit fileSignalManager->deviceAdded(*localInfo);
        }
    }
    else if (strcmp(action, "remove") == 0)
    {
        DeviceInfo * localInfo = removeDevice(dev);
        if(localInfo)
        {
            qDebug() << "device removed";
            emit fileSignalManager->deviceRemoved(*localInfo);
            delete localInfo;
        }
    }

    udev_device_unref(dev);
}

void DeviceListener::mount(const QString &path)
{
    Q_UNUSED(path)
//    QStringList r = path.split("/");

//    udev_device *dev = udev_device_new_from_syspath(m_udev, path.toLatin1());
//    QString label = deviceLabel(dev);

//    QProcess p;
//    p.start("/usr/bin/pmount", QStringList() << "/dev/" + r.last());
//    p.waitForStarted();
//    if (p.waitForFinished() && p.exitCode() == 0)
//    {

//    }
//    else
//    {
//        qDebug() << "mount error" << p.readAllStandardError();
//    }
//    udev_device_unref(dev);
}

void DeviceListener::unmount(const QString &path)
{
    Q_UNUSED(path)
//    QStringList r = path.split("/");

//    udev_device *dev = udev_device_new_from_syspath(m_udev, path.toLatin1());
//    QString label = deviceLabel(dev);

//    QProcess p;
//    p.start("/usr/bin/pumount", QStringList() << "/dev/" + r.last());
//    p.waitForStarted();
//    if (p.waitForFinished() && p.exitCode() == 0)
//    {

//    }
//    else
//    {
//        qDebug() << "unmount error" << p.readAllStandardError();
//    }
//    udev_device_unref(dev);
}

QString DeviceListener::mountPoint(udev_device *dev)
{
    QFile mtab("/etc/mtab");
    mtab.open(QFile::ReadOnly);
    const char *name = udev_device_get_sysname(dev);
    foreach(QString l, QString(mtab.readAll()).split("\n")) {
        QStringList r = l.split(" ");
        if (r.at(0) == QString("/dev/") + name) {
            return r.at(1);
        }
    }
    return QString();
}

const QList<AbstractFileInfoPointer> DeviceListener::getChildren(const DUrl &fileUrl, QDir::Filters filter, const FMEvent &event, bool &accepted) const
{
    Q_UNUSED(fileUrl)
    Q_UNUSED(filter)

    QList<AbstractFileInfoPointer> infolist;

    for (int i = 0; i < m_deviceInfos.size(); i++)
    {
        DeviceInfo * info = m_deviceInfos.at(i);
        AbstractFileInfoPointer fileInfo(new DeviceInfo(info));
        infolist.append(fileInfo);
    }

    accepted = true;

    return infolist;
}

const AbstractFileInfoPointer DeviceListener::createFileInfo(const DUrl &fileUrl, bool &accepted) const
{
    accepted = true;

    return AbstractFileInfoPointer(new DeviceInfo(fileUrl));
}
