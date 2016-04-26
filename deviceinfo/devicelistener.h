#ifndef DEVICELISTENER_H
#define DEVICELISTENER_H

#include <QDebug>
#include <QSocketNotifier>
#include <QApplication>
#include <QProcess>
#include <QFile>
#include <QStorageInfo>
#include <QMap>

#include <libudev.h>
#include <stdio.h>
#include <sys/vfs.h>
#include <fcntl.h>

#include "../filemanager/controllers/abstractfilecontroller.h"

struct udev_device;
struct udev_monitor;
struct udev;

class DeviceInfo;

class DeviceListener : public AbstractFileController
{
    Q_OBJECT
public:
    DeviceListener();
    ~DeviceListener();
    QList<DeviceInfo *> loadDeivces();
private:
    udev *m_udev;
    udev_monitor *m_udevMonitor;
    QList<DeviceInfo *> m_deviceInfos;
    QMap<QString, DeviceInfo *> m_deviceMap;

    bool isTargetDevice(udev_device *dev);
    QString mountpoint(udev_device *dev);
    QString deviceLabel(udev_device *dev);
    QString deviceUUID(udev_device *dev);
    DeviceInfo * addDevice(udev_device *dev);
    DeviceInfo * removeDevice(udev_device *dev);

public slots:
    void deviceReceived(int);
    void mount(const QString &path);
    void unmount(const QString &path);
    QString mountPoint(udev_device *dev);
signals:
    void deviceAdded(const DeviceInfo &deviceInfos);
    void deviceRemoved(const DeviceInfo &deviceInfos);

    // AbstractFileController interface
public:
    const QList<AbstractFileInfoPointer> getChildren(const DUrl &fileUrl, QDir::Filters filter, bool &accepted) const Q_DECL_OVERRIDE;
    AbstractFileInfoPointer createFileInfo(const DUrl &fileUrl, bool &accepted) const Q_DECL_OVERRIDE;
};

#endif // DEVICELISTENER_H
