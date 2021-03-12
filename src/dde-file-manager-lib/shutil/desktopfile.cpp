/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "desktopfile.h"
#include "properties.h"
#include <QFile>
#include <QSettings>
#include <QDebug>

/**
 * @brief Loads desktop file
 * @param fileName
 */
DesktopFile::DesktopFile(const QString &fileName)
    : m_fileName(fileName)
{
    // File validity
    if (m_fileName.isEmpty() || !QFile::exists(fileName)) {
        return;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(fileName, "Desktop Entry");

    if (desktop.contains("X-Deepin-AppID")) {
        m_deepinId = desktop.value("X-Deepin-AppID", settings.value("X-Deepin-AppID")).toString();
    }

    if (desktop.contains("X-Deepin-Vendor")) {
        m_deepinVendor = desktop.value("X-Deepin-Vendor", settings.value("X-Deepin-Vendor")).toString();
    }

    if (desktop.contains("NoDisplay")) {
        m_noDisplay = desktop.value("NoDisplay", settings.value("NoDisplay").toBool()).toBool();
    }
    if (desktop.contains("Hidden")) {
        m_hidden = desktop.value("Hidden", settings.value("Hidden").toBool()).toBool();
    }

    //由于获取的系统语言简写与.desktop的语言简写存在不对应关系，经决定先采用获取的系统值匹配
    //若没匹配到则采用系统值"_"左侧的字符串进行匹配，均为匹配到，才走原未匹配流程
    auto getValueFromSys = [&desktop, &settings](const QString & type, const QString & sysName)->QString {
        const QString key = QString("%0[%1]").arg(type).arg(sysName);
        return desktop.value(key, settings.value(key)).toString();
    };

    auto getNameByType = [&desktop, &settings, &getValueFromSys](const QString & type)->QString{
        QString tempSysName = QLocale::system().name();
        QString targetName = getValueFromSys(type, tempSysName);
        if (targetName.isEmpty())
        {
            auto strSize = tempSysName.trimmed().split("_");
            if (!strSize.isEmpty()) {
                tempSysName = strSize.first();
                targetName = getValueFromSys(type, tempSysName);
            }

            if (targetName.isEmpty())
                targetName = desktop.value(type, settings.value(type)).toString();
        }

        return targetName;
    };
    m_localName = getNameByType("Name");
    m_genericName = getNameByType("GenericName");

    m_exec = desktop.value("Exec", settings.value("Exec")).toString();
    m_icon = desktop.value("Icon", settings.value("Icon")).toString();
    m_type = desktop.value("Type", settings.value("Type", "Application")).toString();
    m_categories = desktop.value("Categories", settings.value("Categories").toString()).toString().remove(" ").split(";");

    QString mime_type = desktop.value("MimeType", settings.value("MimeType").toString()).toString().remove(" ");

    if (!mime_type.isEmpty())
        m_mimeType = mime_type.split(";");
    // Fix categories
    if (m_categories.first().compare("") == 0) {
        m_categories.removeFirst();
    }
}
//---------------------------------------------------------------------------

QString DesktopFile::getFileName() const
{
    return m_fileName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getPureFileName() const
{
    return m_fileName.split("/").last().remove(".desktop");
}
//---------------------------------------------------------------------------

QString DesktopFile::getName() const
{
    return m_name;
}

QString DesktopFile::getLocalName() const
{
    return m_localName;
}

QString DesktopFile::getDisplayName() const
{
    if (m_deepinVendor == QStringLiteral("deepin") && !m_genericName.isEmpty()) {
        return m_genericName;
    }
    return m_localName.isEmpty() ? m_name : m_localName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getExec() const
{
    return m_exec;
}
//---------------------------------------------------------------------------

QString DesktopFile::getIcon() const
{
    return m_icon;
}
//---------------------------------------------------------------------------

QString DesktopFile::getType() const
{
    return m_type;
}

QString DesktopFile::getDeepinId() const
{
    return m_deepinId;
}

QString DesktopFile::getDeepinVendor() const
{
    return m_deepinVendor;
}

bool DesktopFile::getNoShow() const
{
    return m_noDisplay || m_hidden;
}

//---------------------------------------------------------------------------

QStringList DesktopFile::getCategories() const
{
    return m_categories;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getMimeType() const
{
    return m_mimeType;
}
//---------------------------------------------------------------------------
