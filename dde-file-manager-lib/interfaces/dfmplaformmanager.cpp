/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

bool DFMPlaformManager::isCompactList() const
{
    m_platformSettings->beginGroup("x86_pro");

    bool value = m_platformSettings->value("compact_list", "false").toString() == "true";

    m_platformSettings->endGroup();

    return value;
}

bool DFMPlaformManager::isAutoCompactList() const
{
    m_platformSettings->beginGroup("x86_pro");

    bool value = m_platformSettings->value("auto_compact_list", "false").toString() == "true";

    m_platformSettings->endGroup();

    return value;
}
