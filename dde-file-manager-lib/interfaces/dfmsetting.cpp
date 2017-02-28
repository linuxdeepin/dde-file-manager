#include "dfmsetting.h"
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTemporaryFile>
#include "dfmstandardpaths.h"
#include <qsettingbackend.h>
#include <settings.h>
#include "../widgets/singleton.h"
#include "app/filesignalmanager.h"
#include "interfaces/dfileservices.h"
#include "interfaces/dabstractfilewatcher.h"
#include "app/define.h"

DFMSetting::DFMSetting(QObject *parent) : QObject(parent)
{
    paths << "Current Path"
          << DFMStandardPaths::standardLocation(DFMStandardPaths::HomePath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::DesktopPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::VideosPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::MusicPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::PicturesPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::DocumentsPath)
          << DFMStandardPaths::standardLocation(DFMStandardPaths::DownloadsPath);

    //load temlate
    m_settings = Settings::fromJsonFile(":/configure/global-setting-template.json").data();

    //load conf value
    auto backen = new QSettingBackend(getConfigFilePath());
    m_settings->setBackend(backen);

    m_fileSystemWathcer = fileService->createFileWatcher(DUrl::fromLocalFile(getConfigFilePath()).parentUrl());
    m_fileSystemWathcer->startWatcher();

    initConnections();
}

void DFMSetting::initConnections()
{
    connect(m_settings, &Dtk::Settings::valueChanged, this, &DFMSetting::onValueChanged);
    connect(m_fileSystemWathcer, &DAbstractFileWatcher::fileMoved, this, &DFMSetting::onConfigFileChanged);
}

QVariant DFMSetting::getValueByKey(const QString &key)
{
    return m_settings->value(key);
}

bool DFMSetting::isAllwayOpenOnNewWindow()
{
    if (DFMGlobal::IsFileManagerDiloagProcess){
        return false;
    }
    return getValueByKey("base.open_action.allways_open_on_new_window").toBool();
}

int DFMSetting::iconSizeIndex()
{
    return getValueByKey("base.default_view.icon_size").toInt();
}

int DFMSetting::openFileAction()
{
    if (DFMGlobal::IsFileManagerDiloagProcess){
        return 1;
    }
    const int& index = getValueByKey("base.open_action.open_file_action").toInt();
    return index;
}

QString DFMSetting::newWindowPath()
{
    const int& index = getValueByKey("base.new_tab_windows.new_window_path").toInt();
    if(index < paths.count() && index >= 0)
        return paths[index];
    return "Current Path";
}

QString DFMSetting::newTabPath()
{
    const int& index = getValueByKey("base.new_tab_windows.new_tab_path").toInt();
    if(index < paths.count() && index >= 0)
        return paths[index];
    return "Current Path";
}

QString DFMSetting::getConfigFilePath()
{
    return QString("%1/%2").arg(DFMStandardPaths::getConfigPath(), "dde-file-manager.conf");
}

QPointer<Settings> DFMSetting::settings()
{
    return m_settings;
}

void DFMSetting::onValueChanged(const QString &key, const QVariant &value)
{
    Q_UNUSED(value);
    QStringList reactKeys;
    reactKeys << "advance.preview.text_file_preview"
              << "advance.preview.document_file_preview"
              << "advance.preview.image_file_preview"
              << "advance.preview.video_file_preview"
              << "base.default_view.icon_size";
    if(reactKeys.contains(key)){
        emit fileSignalManager->requestChangeIconSizeBySizeIndex(iconSizeIndex());
        emit fileSignalManager->requestFreshAllFileView();
    }
}

void DFMSetting::onConfigFileChanged(const DUrl &fromUrl, const DUrl &toUrl)
{
    Q_UNUSED(fromUrl)
    if (toUrl == DUrl::fromLocalFile(getConfigFilePath())){
        auto backen = new QSettingBackend(getConfigFilePath());
        m_settings->setBackend(backen);
        qDebug() << toUrl;
    }
}

bool DFMSetting::isQuickSearch()
{
    return getValueByKey("advance.search.quick_search").toBool();
}

bool DFMSetting::isCompressFilePreview()
{
    return getValueByKey("advance.preview.compress_file_preview").toBool();
}

bool DFMSetting::isTextFilePreview()
{
    return getValueByKey("advance.preview.text_file_preview").toBool();
}

bool DFMSetting::isDocumentFilePreview()
{
    return getValueByKey("advance.preview.document_file_preview").toBool();
}

bool DFMSetting::isImageFilePreview()
{
    return getValueByKey("advance.preview.image_file_preview").toBool();
}

bool DFMSetting::isVideoFilePreview()
{
    return getValueByKey("advance.preview.video_file_preview").toBool();
}

bool DFMSetting::isAutoMount()
{
    return getValueByKey("advance.mount.auto_mount").toBool();
}

bool DFMSetting::isAutoMountAndOpen()
{
    return getValueByKey("advance.mount.auto_mount_and_open").toBool();
}

bool DFMSetting::isDefaultChooserDialog()
{
    return getValueByKey("advance.dialog.default_chooser_dialog").toBool();
}

bool DFMSetting::isShowedHiddenOnSearch()
{
    return getValueByKey("advance.search.show_hidden").toBool();
}

bool DFMSetting::isShowedHiddenOnView()
{
    return getValueByKey("base.hidden_files.show_hidden").toBool();
}
