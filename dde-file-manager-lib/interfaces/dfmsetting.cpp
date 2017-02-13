#include "dfmsetting.h"
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTemporaryFile>
#include "dfmstandardpaths.h"
#include <qsettingbackend.h>

DFMSetting::DFMSetting(QObject *parent) : QObject(parent)
{
    paths << "Current Path"
          << DFMStandardPaths::standardLocation(DFMStandardPaths::HomePath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::DesktopPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::VideosPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::MusicPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::PicturesPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::DocumentsPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::DownloadsPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::ComputerRoorPath)
          << "/"
          << DFMStandardPaths::standardLocation(DFMStandardPaths::NetworkRootPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::UserShareRootPath);

    m_settings = Settings::fromJsonFile(":/configure/global-setting-template.json");
    auto backen = new QSettingBackend(getConfigFilePath());
    m_settings->setBackend(backen);
}

bool DFMSetting::isAllwayOpenOnNewWindow()
{
    return m_settings->value("base.new_tab_windows.alway_open_on_new").toBool();
}

int DFMSetting::iconSizeIndex()
{
    return m_settings->value("base.default_view.icon_size").toInt();
}

int DFMSetting::openFileAction()
{
    const int& index = m_settings->value("base.open_action.open_file_action").toInt();
    return index;
}

QString DFMSetting::newWindowPath()
{
    const int& index = m_settings->value("base.new_tab_windows.new_window_path").toInt();
    if(index < paths.count() && index >= 0)
        return paths[index];
    return "Current Path";
}

QString DFMSetting::newTabPath()
{
    const int& index = m_settings->value("base.new_tab_windows.new_tab_path").toInt();
    if(index < paths.count() && index >= 0)
        return paths[index];
    return "Current Path";
}

QString DFMSetting::getConfigFilePath()
{
    return QString("%1/%2").arg(DFMStandardPaths::getConfigPath(), "dde-file-manager.conf");
}

void DFMSetting::setSettings(Settings *settings)
{
    m_settings = settings;
}
