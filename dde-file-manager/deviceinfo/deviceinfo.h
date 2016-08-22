#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>
#include "models/abstractfileinfo.h"

#define BYTES_PER_BLOCK 512

class DeviceInfo : public AbstractFileInfo
{
public:
    DeviceInfo(DeviceInfo * deviceInfo);
    DeviceInfo(const DUrl &url);
    DeviceInfo(const QString &path,
               const QString &sysPath,
               const QString &label,
               const QString &devLabel,
               const QString &uuid,
               const QString &size);
    QString getMountPath();
    QString getLabel();
    QString getSysPath();
    QString getDeviceLabel();
    QString getUUID();
    QString getSize();
private:
    QString m_mountPath;
    QString m_label;
    QString m_sysPath;
    QString m_devLabel;
    QString m_uuid;
    QString m_size;

    QString formatSize( qint64 num ) const;

    // AbstractFileInfo interface
public:
    QString displayName() const Q_DECL_OVERRIDE;
    bool isCanRename() const Q_DECL_OVERRIDE;
    bool isDir() const Q_DECL_OVERRIDE;
    QIcon fileIcon() const Q_DECL_OVERRIDE;
};

#endif // DEVICEINFO_H
