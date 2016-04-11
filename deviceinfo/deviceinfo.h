#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>


class DeviceInfo
{
public:
    DeviceInfo(const QString &path,
               const QString &sysPath,
               const QString &label,
               const QString &devLabel);
    QString getMountPath();
    QString getLabel();
    QString getSysPath();
    QString getDeviceLabel();
private:
    QString m_mountPath;
    QString m_label;
    QString m_sysPath;
    QString m_devLabel;
};

#endif // DEVICEINFO_H
