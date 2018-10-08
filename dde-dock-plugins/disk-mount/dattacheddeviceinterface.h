#ifndef DATTACHEDDEVICEINTERFACE_H
#define DATTACHEDDEVICEINTERFACE_H

#include <QString>
#include <QPair>
#include <QUrl>

class DAttachedDeviceInterface {
public:
    virtual bool detachable() = 0; // can be unmounted / removable or not.
    virtual void detach() = 0; // do unmount, also do eject if possible.
    virtual QString displayName() = 0; // device display name.
    virtual bool deviceUsageValid() = 0; // storage valid / available or not.
    virtual QPair<quint64, quint64> deviceUsage() = 0; // free / total, in bytes.
    virtual QString iconName() = 0; // device icon (theme) name.
    virtual QUrl mountpointUrl() = 0; // as name.
};

#endif // DATTACHEDDEVICEINTERFACE_H
