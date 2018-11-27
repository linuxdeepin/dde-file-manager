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

#include "desktopfile.h"
#include "properties.h"
#include <QFile>
#include <QSettings>
#include <QDebug>

/**
 * @brief Loads desktop file
 * @param fileName
 */
DesktopFile::DesktopFile(const QString &fileName) {

    // Store file name
    m_fileName = fileName;

    // File validity
    if (m_fileName.isEmpty() || !QFile::exists(fileName)) {
        return;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(fileName, "Desktop Entry");
    m_name = desktop.value("Name", settings.value("Name")).toString();
    m_genericName = desktop.value("GenericName", settings.value("GenericName")).toString();

    if(desktop.contains("X-Deepin-AppID")){
        m_deepinId = desktop.value("X-Deepin-AppID", settings.value("X-Deepin-AppID")).toString();
    }

    if(desktop.contains("X-Deepin-Vendor")){
        m_deepinVendor = desktop.value("X-Deepin-Vendor", settings.value("X-Deepin-Vendor")).toString();
    }

    QString nLocalKey = QString("Name[%1]").arg(QLocale::system().name());
    if (desktop.contains(nLocalKey)){
        m_localName = desktop.value(nLocalKey, m_name).toString();
    }else{
        m_localName = m_name;
    }

    QString gnlocalKey = QString("GenericName[%1]").arg(QLocale::system().name());
    if (desktop.contains(gnlocalKey)){
        m_genericName = desktop.value(gnlocalKey, m_name).toString();
    }else{
        m_genericName = m_name;
    }

    if(desktop.contains("NoDisplay")){
        m_noDisplay = desktop.value("NoDisplay", settings.value("NoDisplay").toBool()).toBool();
    }
    if(desktop.contains("Hidden")){
        m_hidden = desktop.value("Hidden", settings.value("Hidden").toBool()).toBool();
    }

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

QString DesktopFile::getFileName() const {
    return m_fileName;
}
//---------------------------------------------------------------------------

QString DesktopFile::getPureFileName() const {
    return m_fileName.split("/").last().remove(".desktop");
}
//---------------------------------------------------------------------------

QString DesktopFile::getName() const {
    return m_name;
}

QString DesktopFile::getLocalName() const {
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

QString DesktopFile::getExec() const {
    return m_exec;
}
//---------------------------------------------------------------------------

QString DesktopFile::getIcon() const {
    return m_icon;
}
//---------------------------------------------------------------------------

QString DesktopFile::getType() const {
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

QStringList DesktopFile::getCategories() const {
    return m_categories;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getMimeType() const {
    return m_mimeType;
}
//---------------------------------------------------------------------------
