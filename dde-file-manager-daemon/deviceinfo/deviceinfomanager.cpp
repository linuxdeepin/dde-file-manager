#include "deviceinfomanager.h"
#include "dbusadaptor/deviceinfomanager_adaptor.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QtConcurrent>
#include <QThreadPool>
#include <QFuture>
#include <QDebug>

QString DeviceInfoManager::ObjectPath = "/com/deepin/filemanager/daemon/DeviceInfoManager";
DeviceInfoManager::DeviceInfoManager(QObject *parent) : QObject(parent)
{
    PartMan::Partition::registerMetaType();
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_deviceInfoManagerAdaptor = new DeviceInfoManagerAdaptor(this);
    m_readUsageManager = new PartMan::ReadUsageManager(this);
}

PartMan::Partition DeviceInfoManager::getPartitionByDevicePath(const QString &devicePath)
{
    PartMan::Partition p = PartMan::Partition::getPartitionByDevicePath(devicePath);
    qDebug() << p;
    return p;
}

bool DeviceInfoManager::readUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
//    typedef bool (PartMan::ReadUsageManager::*readUsageFun) (const QString&, qlonglong& , qlonglong&);
//    readUsageFun f = &PartMan::ReadUsageManager::readUsage;
//    QFuture<bool> future = QtConcurrent::run(m_readUsageManager,
//                                             f,
//                                             path, freespace, total);
//    future.waitForFinished();
//    bool ret = future.result();
    bool ret = m_readUsageManager->readUsage(path , freespace, total);
    return ret;
}
