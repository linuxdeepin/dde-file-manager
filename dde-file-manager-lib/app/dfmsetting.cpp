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

#include "app/dfmsetting.h"

#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QProcess>

#include <DSettings>
#include <qsettingbackend.h>

#include "dfmstandardpaths.h"
#include "singleton.h"
#include "app/filesignalmanager.h"
#include "interfaces/dfileservices.h"
#include "interfaces/dabstractfilewatcher.h"
#include "app/define.h"
#include "shutil/fileutils.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

DCORE_USE_NAMESPACE

class SettingBackend : public DSettingsBackend
{
public:
    explicit SettingBackend(DFMSettings *fms);

    QStringList keys() const;
    QVariant getOption(const QString &key) const;

    void doSync();

protected:
    void doSetOption(const QString &key, const QVariant &value);

private:
    QString group = QStringLiteral("settings dialog");

    DFMSettings *m_setting;
};

SettingBackend::SettingBackend(DFMSettings *fms)
    : m_setting(fms)
{
    connect(m_setting, &DFMSettings::valueChanged, this, [this] (const QString &group, const QString &key, const QVariant &value) {
        if (group != this->group)
            return;

        emit optionChanged(key, value);
    });

    m_setting->setAutoSync(true);
    m_setting->setWatchChanges(true);
}

QStringList SettingBackend::keys() const
{
    return m_setting->keys(group).toList();
}

QVariant SettingBackend::getOption(const QString &key) const
{
    return m_setting->value(group, key);
}

void SettingBackend::doSync()
{
    m_setting->sync();
}

void SettingBackend::doSetOption(const QString &key, const QVariant &value)
{
    m_setting->setValue(group, key, value);
}

DFMSetting *DFMSetting::instance()
{
    return globalSetting;
}

static auto fromJsJson(const QString &fileName) -> decltype(DSettings::fromJson(QByteArray()))
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly)) {
        return nullptr;
    }

    QByteArray data = file.readAll();

    file.close();

    auto indexOfChar = [] (const QByteArray &data, char ch, int from) {
        for (; from < data.size(); ++from) {
            if (data.at(from) == '\\')
                continue;

            if (data.at(from) == ch)
                return from;
        }

        return from;
    };

    auto clean_qsTr = [indexOfChar] (QByteArray &data, int &from) {
        const QByteArray &qsTr = QByteArrayLiteral("qsTr");
        const QByteArray &qsTranslate = QByteArrayLiteral("anslate(");

        if (qsTr != QByteArray(data.data() + from, qsTr.size()))
            return;

        int index = from + qsTr.size();

        if (data.at(index) == '(') {
            data.remove(from, index - from + 1);
        } else if (qsTranslate == QByteArray(data.data() + index, qsTranslate.size())) {
            index += qsTranslate.size();

            // 寻找qsTranslate的第一个参数
            if (data.at(index) == '"' || data.at(index) == '\'') {
                index = indexOfChar(data, data.at(index), index + 1);

                if (index >= data.size())
                    return;
            } else {
                return;
            }

            int quote1_index = data.indexOf('"', index + 1);
            int quote2_index = data.indexOf('\'', index + 1);

            if (quote1_index > 0)
                index = quote1_index;

            if (quote2_index > 0 && quote2_index < index)
                index = quote2_index;

            data.remove(from, index - from);
        } else {
            return;
        }

        // 保留需要翻译的字符串
        if (data.at(from) == '"' || data.at(from) == '\'') {
            from = indexOfChar(data, data.at(from), from + 1);

            if (from >= data.size())
                return;
        } else {
            return;
        }

        from = indexOfChar(data, ')', from + 1);

        if (from < data.size()) {
            data.remove(from, 1);
            from -= 1;
        }
    };

    for (int i = 0; i < data.size(); ++i) {
        char ch = data.at(i);

        switch (ch) {
        case '\\':
            break;
        case '\'':
        case '"':
            i = indexOfChar(data, ch, i + 1);
            break;
        case 'q':
            clean_qsTr(data, i);
            break;
        default:
            break;
        }
    }

    return DSettings::fromJson(data);
}

DFMSetting::DFMSetting(QObject *parent) : QObject(parent)
{
    m_newTabOptionPaths
            << "Current Path"
            << DFMStandardPaths::standardLocation(DFMStandardPaths::ComputerRootPath)
            << DFMStandardPaths::standardLocation(DFMStandardPaths::HomePath)
            << DFMStandardPaths::standardLocation(DFMStandardPaths::DesktopPath)
            << DFMStandardPaths::standardLocation(DFMStandardPaths::VideosPath)
            << DFMStandardPaths::standardLocation(DFMStandardPaths::MusicPath)
            << DFMStandardPaths::standardLocation(DFMStandardPaths::PicturesPath)
            << DFMStandardPaths::standardLocation(DFMStandardPaths::DocumentsPath)
            << DFMStandardPaths::standardLocation(DFMStandardPaths::DownloadsPath);

    m_defaultWindowOptionPaths = m_newTabOptionPaths;
    m_defaultWindowOptionPaths.removeFirst();


#ifdef DISABLE_COMPRESS_PREIVEW
    //load temlate
    m_settings = fromJsJson(":/configure/global-setting-template-pro.js").data();
#else
#ifndef SUPPORT_FFMEPG
    m_settings = fromJsJson(":/configure/global-setting-template-fedora.js").data();
#else
    m_settings = fromJsJson(":/configure/global-setting-template.js").data();
#endif
#endif

    //load conf value
    auto backen = new SettingBackend(DFMApplication::genericSetting());

    m_settings->setBackend(backen);

    initConnections();
}

void DFMSetting::initConnections()
{
    connect(m_settings, &Dtk::Core::DSettings::valueChanged, this, &DFMSetting::onValueChanged);
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

int DFMSetting::viewMode()
{
    return getValueByKey("base.default_view.view_mode").toInt() + 1;
}

int DFMSetting::openFileAction()
{
    if (DFMGlobal::IsFileManagerDiloagProcess){
        return 1;
    }
    const int& index = getValueByKey("base.open_action.open_file_action").toInt();
    return index;
}

QString DFMSetting::defaultWindowPath()
{
    const int& index = getValueByKey("base.new_tab_windows.default_window_path").toInt();
    if(index < m_defaultWindowOptionPaths.count() && index >= 0)
        return m_defaultWindowOptionPaths[index];
    return DFMStandardPaths::standardLocation(DFMStandardPaths::HomePath);
}

QString DFMSetting::newTabPath()
{
    const int& index = getValueByKey("base.new_tab_windows.new_tab_path").toInt();
    if(index < m_newTabOptionPaths.count() && index >= 0)
        return m_newTabOptionPaths[index];
    return "Current Path";
}

QString DFMSetting::getConfigFilePath()
{
    return QString("%1/%2").arg(DFMStandardPaths::getConfigPath(), "dde-file-manager.conf");
}

QPointer<Dtk::Core::DSettings> DFMSetting::settings()
{
    return m_settings;
}

void DFMSetting::onValueChanged(const QString &key, const QVariant &value)
{
    Q_UNUSED(value);
    QStringList previewKeys;
    previewKeys << "advance.preview.text_file_preview"
              << "advance.preview.document_file_preview"
              << "advance.preview.image_file_preview"
              << "advance.preview.video_file_preview";

    if(previewKeys.contains(key)){
        emit fileSignalManager->requestFreshAllFileView();
    } else if(key == "base.default_view.icon_size"){
        emit fileSignalManager->requestChangeIconSizeBySizeIndex(iconSizeIndex());
    } else if(key == "base.hidden_files.show_hidden"){
        emit fileSignalManager->showHiddenOnViewChanged();
    } else if (key == "advance.preview.compress_file_preview"){
        if (value.toBool()){
            FileUtils::mountAVFS();
        }else{
            FileUtils::umountAVFS();
        }
    }else if (key == "base.default_view.view_mode"){
        emit fileSignalManager->defaultViewModeChanged(viewMode());
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

bool DFMSetting::isShowedFileSuffix() noexcept
{
    return getValueByKey("base.hidden_files.hide_suffix").toBool();
}


