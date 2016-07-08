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
    if (!QFile::exists(fileName)) {
        return;
    }

    QSettings settings(fileName, QSettings::IniFormat);
    settings.beginGroup("Desktop Entry");
    // Loads .desktop file (read from 'Desktop Entry' group)
    Properties desktop(fileName, "Desktop Entry");
    m_name = desktop.value("Name", settings.value("Name")).toString();


    QString localKey = QString("Name[%1]").arg(QLocale::system().name());
    if (desktop.contains(localKey)){
        m_localName = desktop.value(localKey, m_name).toString();
    }else{
        m_localName = m_name;
    }
    m_exec = desktop.value("Exec", settings.value("Exec")).toString();
    m_icon = desktop.value("Icon", settings.value("Icon")).toString();
    m_type = desktop.value("Type", settings.value("Type", "Application")).toString();
    m_categories = desktop.value("Categories", settings.value("Categories").toString()).toString().remove(" ").split(";");
    m_mimeType = desktop.value("MimeType", settings.value("MimeType").toString()).toString().remove(" ").split(";");
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
//---------------------------------------------------------------------------

QStringList DesktopFile::getCategories() const {
    return m_categories;
}
//---------------------------------------------------------------------------

QStringList DesktopFile::getMimeType() const {
    return m_mimeType;
}
//---------------------------------------------------------------------------
