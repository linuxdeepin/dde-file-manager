#include "dfmplaformmanager.h"
#include "dfmstandardpaths.h"
#include <QDebug>
#include <QFile>

DFMPlaformManager::DFMPlaformManager(QObject *parent) : QObject(parent)
{
    m_platformSettings = new QSettings(platformConfigPath(), QSettings::IniFormat);
}

QString DFMPlaformManager::platformConfigPath()
{
    return QString("%1/config/dfm-platform.conf").arg(DFMStandardPaths::standardLocation(DFMStandardPaths::ApplicationSharePath));
}

bool DFMPlaformManager::isDisableUnMount()
{
    bool ret = false;
    m_platformSettings->beginGroup("x86_pro");
    if (m_platformSettings->value("disable_mount").toString() == "true"){
        ret = true;
    }
    m_platformSettings->endGroup();

    return ret;
}

bool DFMPlaformManager::isRoot_hidden()
{
    bool ret = false;
    m_platformSettings->beginGroup("x86_pro");
    if (m_platformSettings->value("root_hidden").toString() == "true"){
        ret = true;
    }
    m_platformSettings->endGroup();

    return ret;
}
