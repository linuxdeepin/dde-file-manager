#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>
#include "../filemanager/models/abstractfileinfo.h"


class DeviceInfo : public AbstractFileInfo
{
public:
    DeviceInfo(DeviceInfo * deviceInfo);
    DeviceInfo(const QString &url);
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
    QString displayName() const;
    bool isCanRename() const;
    bool isDir() const;
    QIcon fileIcon() const;
};

#endif // DEVICEINFO_H
