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
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_deviceInfoManagerAdaptor = new DeviceInfoManagerAdaptor(this);
    m_readUsageManager = new PartMan::ReadUsageManager(this);
}

bool DeviceInfoManager::readUsage(const QString &path, qlonglong &freespace, qlonglong &total)
{
    typedef bool (PartMan::ReadUsageManager::*readUsageFun) (const QString&, qlonglong& , qlonglong&);
    readUsageFun f = &PartMan::ReadUsageManager::readUsage;
    QFuture<bool> future = QtConcurrent::run(m_readUsageManager,
                                             f,
                                             path, freespace, total);
    future.waitForFinished();
    bool ret = future.result();
    return ret;
}
