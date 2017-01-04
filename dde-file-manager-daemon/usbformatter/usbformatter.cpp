#include "usbformatter.h"
#include "dbusservice/dbusadaptor/usbformatter_adaptor.h"

#include <QDBusConnection>
#include <QDBusVariant>
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
    return m_partitionManager->mkfs(path, fs, label);
}
