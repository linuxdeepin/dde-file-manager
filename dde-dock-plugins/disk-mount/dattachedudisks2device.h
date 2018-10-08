#ifndef DATTACHEDUDISKS2DEVICE_H
#define DATTACHEDUDISKS2DEVICE_H

#include "dattacheddeviceinterface.h"

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE
class DFMBlockDevice;
DFM_END_NAMESPACE

class DAttachedUdisks2Device : public DAttachedDeviceInterface
{
public:
    DAttachedUdisks2Device(const DFM_NAMESPACE::DFMBlockDevice *blockDevicePointer);

    bool detachable() override;
    void detach() override;
    QString displayName() override;
    bool deviceUsageValid() override;
    QPair<quint64, quint64> deviceUsage() override;
    QString iconName() override;
    QUrl mountpointUrl() override;

    DFM_NAMESPACE::DFMBlockDevice* blockDevice();

private:
    QScopedPointer<DFM_NAMESPACE::DFMBlockDevice> c_blockDevice;
    QString deviceDBusId;
    QString mountPoint;

    const QString ddeI18nSym = QStringLiteral("_dde_");
};

#endif // DATTACHEDUDISKS2DEVICE_H
