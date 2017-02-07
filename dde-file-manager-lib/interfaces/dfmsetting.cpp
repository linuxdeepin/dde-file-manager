#include "dfmsetting.h"
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTemporaryFile>
#include "dfmstandardpaths.h"

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
}

void DFMSetting::loadConfigDataFromJsonFile(const QString &filePath)
{

    if(!QFile::exists(filePath)){
        reCreateConfigTemplate(filePath);
    }

    settings = Settings::fromJsonFile(filePath);
}

void DFMSetting::reCreateConfigTemplate(const QString &filePath)
{
    QByteArray jsonData;
    QFile templateFile(":/configure/global-setting-template.json");
    if(!templateFile.open(QIODevice::ReadOnly)){
        qDebug () << "read configure json template error:" << templateFile.errorString();
        return ;
    }

    jsonData = templateFile.readAll();
    templateFile.close();

    QTemporaryFile::createNativeFile(filePath);
    QFile tempFile(filePath);
    if(!tempFile.open(QIODevice::ReadWrite|QIODevice::Text)){
        qDebug () << "fail to open file :" << filePath;
        return ;
    }

    qDebug () << "write data:" << jsonData;

    QTextStream ts(&tempFile);
    ts << jsonData;
    tempFile.close();
}

bool DFMSetting::isAllwayOpenOnNewWindow()
{
    settings->value("base.new_tab_windows.alway_open_on_new").toBool();
}

int DFMSetting::iconSizeIndex()
{
    return settings->value("base.default_view.icon_size").toInt();
}

DFMSetting::OpenFileAction DFMSetting::openFileAction()
{
    const int& index = settings->value("base.open_action.open_file_action").toInt();
    switch (index) {
    case 0:
        return Click;
    case 1:
        return DoubleClick;
    default:
        return DoubleClick;
    }
    return DoubleClick;
}

QString DFMSetting::newWindowPath()
{
    const int& index = settings->value("base.new_tab_windows.new_window_path").toInt();
    if(index < paths.count() && index >= 0)
        return paths[index];
    return QDir::homePath();
}

QString DFMSetting::newTabPath()
{
    const int& index = settings->value("base.new_tab_windows.new_tab_path").toInt();
    if(index < paths.count() && index >= 0)
        return paths[index];
    return QDir::homePath();
}

DFileView::ViewMode DFMSetting::viewMode()
{
    return DFileView::IconMode;
}
