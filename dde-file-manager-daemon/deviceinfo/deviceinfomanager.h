#ifndef DEVICEINFOMANAGER_H
#define DEVICEINFOMANAGER_H

#include <QObject>
#include "../partman/readusagemanager.h"
#include "../partman/partition.h"

class DeviceInfoManagerAdaptor;


class DeviceInfoManager : public QObject
{
    Q_OBJECT
public:
    static QString ObjectPath;
    explicit DeviceInfoManager(QObject *parent = 0);

signals:

public slots:
    PartMan::Partition getPartitionByDevicePath(const QString& devicePath);
    bool readUsage(const QString &path, qlonglong &freespace, qlonglong &total);

private:
    DeviceInfoManagerAdaptor* m_deviceInfoManagerAdaptor = NULL;
    PartMan::ReadUsageManager* m_readUsageManager = NULL;
};

#endif // DEVICEINFOMANAGER_H
