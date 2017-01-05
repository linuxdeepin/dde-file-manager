#include "usbformatter.h"
#include "dbusservice/dbusadaptor/usbformatter_adaptor.h"
#include "../partman/command.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QtConcurrent>
#include <QThreadPool>
#include <QFuture>
#include <QDebug>

QString UsbFormatter::ObjectPath = "/com/deepin/filemanager/daemon/UsbFormatter";


UsbFormatter::UsbFormatter(QObject *parent) : QObject(parent)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_usbFormatterAdaptor = new UsbFormatterAdaptor(this);
    m_partitionManager = new PartMan::PartitionManager(this);
}

bool UsbFormatter::mkfs(const QString &path, const QString &fs, const QString &label)
{
    typedef bool (PartMan::PartitionManager::*mkfs) (const QString &, const QString & ,const QString &);
    QFuture<bool> future = QtConcurrent::run(QThreadPool::globalInstance(), m_partitionManager,
                                             static_cast<mkfs>(&PartMan::PartitionManager::mkfs),
                                             path, fs, label);
    future.waitForFinished();
    bool ret = future.result();
    return ret;
}
