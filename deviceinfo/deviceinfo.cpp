#include "deviceinfo.h"

DeviceInfo::DeviceInfo(const QString &path, const QString &sysPath, const QString &label, const QString &devLabel)
{
    m_mountPath = path;
    m_label = label;
    m_sysPath = sysPath;
    m_devLabel = devLabel;
}

QString DeviceInfo::getMountPath()
{
    return m_mountPath;
}

QString DeviceInfo::getLabel()
{
    return m_label;
}

QString DeviceInfo::getSysPath()
{
    return m_sysPath;
}

QString DeviceInfo::getDeviceLabel()
{
    return m_devLabel;
}
