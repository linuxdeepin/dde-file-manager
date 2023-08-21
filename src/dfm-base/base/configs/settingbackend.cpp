// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingbackend.h"
#include "private/settingbackend_p.h"
#include "dconfig/dconfigmanager.h"
#include "dfm-base/settingdialog/settingjsongenerator.h"

#include <DSettings>

#include <QDebug>
#include <QApplication>

DFMBASE_USE_NAMESPACE

BidirectionHash<QString, Application::ApplicationAttribute> SettingBackendPrivate::keyToAA {
    { "00_base.00_open_action.00_allways_open_on_new_window", Application::kAllwayOpenOnNewWindow },
    { "00_base.00_open_action.01_open_file_action", Application::kOpenFileMode },
    { "00_base.01_new_tab_windows.00_default_window_path", Application::kUrlOfNewWindow },
    { "00_base.01_new_tab_windows.01_new_tab_path", Application::kUrlOfNewTab },
    { "00_base.02_default_view.00_icon_size", Application::kIconSizeLevel },
    { "00_base.02_default_view.01_view_mode", Application::kViewMode },
    { "00_base.02_default_view.02_mixed_sort", Application::kFileAndDirMixedSort },
    //    { "00_base.02_default_view.view_size_adjustable", Application::kViewSizeAdjustable },
};

BidirectionHash<QString, Application::GenericAttribute> SettingBackendPrivate::keyToGA {
    { "00_base.03_hidden_files.00_show_hidden", Application::kShowedHiddenFiles },
    { "00_base.03_hidden_files.01_show_suffix", Application::kShowedFileSuffix },
    { "01_advance.00_index.00_index_internal", Application::kIndexInternal },
    { "01_advance.00_index.01_index_external", Application::kIndexExternal },
    { "01_advance.00_index.02_index_search", Application::kIndexFullTextSearch },
    //    { "01_advance.search.show_hidden", Application::kShowedHiddenOnSearch },
    { "01_advance.01_preview.00_compress_file_preview", Application::kPreviewCompressFile },
    { "01_advance.01_preview.01_text_file_preview", Application::kPreviewTextFile },
    { "01_advance.01_preview.02_document_file_preview", Application::kPreviewDocumentFile },
    { "01_advance.01_preview.03_image_file_preview", Application::kPreviewImage },
    { "01_advance.01_preview.04_video_file_preview", Application::kPreviewVideo },
    { "01_advance.01_preview.05_audio_file_preview", Application::kPreviewAudio },
    { "01_advance.01_preview.06_remote_env_file_preview", Application::kShowThunmbnailInRemote },
    { "01_advance.02_mount.00_auto_mount", Application::kAutoMount },
    { "01_advance.02_mount.01_auto_mount_and_open", Application::kAutoMountAndOpen },
    { "01_advance.02_mount.02_mtp_show_bottom_info", Application::kMTPShowBottomInfo },
    { "01_advance.02_mount.04_merge_the_entries_of_samba_shared_folders", Application::kMergeTheEntriesOfSambaSharedFolders },
    { "01_advance.03_dialog.00_default_chooser_dialog", Application::kOverrideFileChooserDialog },
    { "01_advance.03_dialog.01_delete_confirmation_dialog", Application::kShowDeleteConfirmDialog },
    { "01_advance.05_other.00_hide_builtin_partition", Application::kHiddenSystemPartition },
    { "01_advance.05_other.02_hide_loop_partitions", Application::kHideLoopPartitions },
    //    { "01_advance.05_other.show_crumbbar_clickable_area", Application::kShowCsdCrumbBarClickableArea },
    { "01_advance.05_other.03_show_filesystemtag_on_diskicon", Application::kShowFileSystemTagOnDiskIcon },
};

SettingBackend::SettingBackend(QObject *parent)
    : DSettingsBackend(parent), d(new SettingBackendPrivate())
{
    Q_ASSERT(Application::instance());

    connect(Application::instance(), &Application::appAttributeEdited, this, &SettingBackend::onValueChanged);
    connect(Application::instance(), &Application::genericAttributeEdited, this, &SettingBackend::onValueChanged);

    initPresetSettingConfig();

    // NOTE(xust): this item is hidden in *template*.js file.
    //    addSettingAccessor(
    //            "01_advance.05_other.extend_file_name",
    //            [] { return DConfigManager::instance()->value(kDefaultCfgPath, "dfm.mount.dlnfs"); },
    //            [](const QVariant &var) { DConfigManager::instance()->setValue(kDefaultCfgPath, "dfm.mount.dlnfs", var); });
}

SettingBackend::~SettingBackend()
{
}

SettingBackend *SettingBackend::instance()
{
    static SettingBackend ins;
    return &ins;
}

void SettingBackend::setToSettings(DSettings *settings)
{
    if (settings) {
        // NOTE: Must move SettingBackend to main thread before call dtk setBackend().
        // The setBackend func will move the SettingBackend object to anthor thread,
        // but the last thread which the SettingBackend object in was destroyed.
        moveToThread(QApplication::instance()->thread());
        settings->setBackend(this);
    }
}

QStringList SettingBackend::keys() const
{
    return d->keyToAA.keys() + d->keyToGA.keys() + d->getters.keys();
}

QVariant SettingBackend::getOption(const QString &key) const
{
    auto &&appVal = d->getAsAppAttr(key);
    if (appVal.isValid())
        return appVal;

    auto &&genVal = d->getAsGenAttr(key);
    if (genVal.isValid())
        return genVal;

    // if a setting item is existed both in settings and dconfig the sametime,
    // return it from settings.
    return d->getByFunc(key);
}

void SettingBackend::doSync()
{
}

void SettingBackend::addSettingAccessor(const QString &key, GetOptFunc get, SaveOptFunc set)
{
    if (set)
        d->setters.insert(key, set);
    else
        qInfo() << "null setter function is passed for key: " << key;
    if (get)
        d->getters.insert(key, get);
    else
        qInfo() << "null getter function is passed for key: " << key;
}

void SettingBackend::addSettingAccessor(Application::ApplicationAttribute attr, SaveOptFunc set)
{
    if (!d->keyToAA.containsValue(attr)) {
        qWarning() << "NO mapped for ApplicationAttr::" << attr;
        return;
    }
    auto uiKey = d->keyToAA.key(attr);
    addSettingAccessor(uiKey, nullptr, set);
}

void SettingBackend::addSettingAccessor(Application::GenericAttribute attr, SaveOptFunc set)
{
    if (!d->keyToGA.containsValue(attr)) {
        qWarning() << "NO map for GenericAttr::" << attr;
        return;
    }
    auto uiKey = d->keyToGA.key(attr);
    addSettingAccessor(uiKey, nullptr, set);
}

void SettingBackend::addToSerialDataKey(const QString &key)
{
    d->serialDataKey.insert(key);
}

void SettingBackend::doSetOption(const QString &key, const QVariant &value)
{
    if (d->serialDataKey.contains(key)) {
        d->saveAsAppAttr(key, value);
        d->saveAsGenAttr(key, value);
        d->saveByFunc(key, value);
    } else {
        QSignalBlocker blocker(this);
        d->saveAsAppAttr(key, value);
        d->saveAsGenAttr(key, value);
        d->saveByFunc(key, value);
    }
}

void SettingBackend::onValueChanged(int attribute, const QVariant &value)
{
    QString key = d->keyToAA.key(static_cast<Application::ApplicationAttribute>(attribute));

    if (key.isEmpty())
        key = d->keyToGA.key(static_cast<Application::GenericAttribute>(attribute));

    if (key.isEmpty())
        return;

    emit optionChanged(key, value);
}

void SettingBackend::initPresetSettingConfig()
{
    auto ins = SettingJsonGenerator::instance();

    // TODO(xust): these configs should be split into plugins.
    ins->addGroup("00_base", "Basic");
    ins->addGroup("00_base.00_open_action", "Open behavior");
    ins->addCheckBoxConfig("00_base.00_open_action.00_allways_open_on_new_window",
                           "Always open folder in new window",
                           false);
    ins->addComboboxConfig("00_base.00_open_action.01_open_file_action",
                           "Open file:",
                           QStringList { "Click",
                                         "Double click" },
                           1);

    ins->addGroup("00_base.01_new_tab_windows", "New window and tab");
    ins->addComboboxConfig("00_base.01_new_tab_windows.00_default_window_path",
                           "Open from default window:",
                           { { "values",
                               QStringList { "Computer",
                                             "Home",
                                             "Desktop",
                                             "Videos",
                                             "Music",
                                             "Pictures",
                                             "Documents",
                                             "Downloads" } },
                             { "keys",
                               QStringList { "computer:///",
                                             "standard://home",
                                             "standard://desktop",
                                             "standard://videos",
                                             "standard://music",
                                             "standard://pictures",
                                             "standard://documents",
                                             "standard://downloads" } } },
                           "computer:///");
    ins->addComboboxConfig("00_base.01_new_tab_windows.01_new_tab_path",
                           "Open in new tab:",
                           { { "values",
                               QStringList { "Current Directory",
                                             "Computer",
                                             "Home",
                                             "Desktop",
                                             "Videos",
                                             "Music",
                                             "Pictures",
                                             "Documents",
                                             "Downloads" } },
                             { "keys",
                               QStringList { "",
                                             "computer:///",
                                             "standard://home",
                                             "standard://desktop",
                                             "standard://videos",
                                             "standard://music",
                                             "standard://pictures",
                                             "standard://documents",
                                             "standard://downloads" } } });

    ins->addGroup("00_base.02_default_view", "View");
    ins->addComboboxConfig("00_base.02_default_view.00_icon_size",
                           "Default size:",
                           QStringList { "Extra small",
                                         "Small",
                                         "Medium",
                                         "Large",
                                         "Extra large" },
                           1);
    ins->addComboboxConfig("00_base.02_default_view.01_view_mode",
                           "Default view:",
                           { { "values", QStringList { "Icon", "List" } },
                             { "keys", QVariantList { 1, 2 } } },
                           1);
    ins->addCheckBoxConfig("00_base.02_default_view.02_mixed_sort",
                           "Mix sorting of files and folders",
                           false);

    ins->addGroup("00_base.03_hidden_files", "Hidden files");
    ins->addCheckBoxConfig("00_base.03_hidden_files.00_show_hidden",
                           "Show hidden files",
                           false);
    ins->addCheckBoxConfig("00_base.03_hidden_files.01_show_suffix",
                           "Show file extensions");

    ins->addGroup("01_advance", "Advanced");
    ins->addGroup("01_advance.00_index", "Index");
    ins->addCheckBoxConfig("01_advance.00_index.00_index_internal",
                           "Auto index internal disk");
    ins->addCheckBoxConfig("01_advance.00_index.01_index_external",
                           "Index external storage device after connected to computer",
                           false);
    ins->addCheckBoxConfig("01_advance.00_index.02_index_search",
                           "Full-Text search",
                           false);

    ins->addGroup("01_advance.01_preview", "Preview");
    ins->addCheckBoxConfig("01_advance.01_preview.00_compress_file_preview",
                           "Compressed file preview",
                           false);
    ins->addCheckBoxConfig("01_advance.01_preview.01_text_file_preview",
                           "Text preview");
    ins->addCheckBoxConfig("01_advance.01_preview.02_document_file_preview",
                           "Document preview");
    ins->addCheckBoxConfig("01_advance.01_preview.03_image_file_preview",
                           "Image preview");
    ins->addCheckBoxConfig("01_advance.01_preview.04_video_file_preview",
                           "Video preview");
    ins->addCheckBoxConfig("01_advance.01_preview.05_audio_file_preview",
                           "Music preview");
    ins->addConfig("01_advance.01_preview.06_remote_env_file_preview",
                   { { "key", "06_remote_env_file_preview" },
                     { "text", "The remote environment shows thumbnail previews" },
                     { "message", "Turning on the thumbnail preview may cause the remote directory to load slowly or the operation to freeze" },
                     { "type", "checkBoxWithMessage" },
                     { "default", false } });

    ins->addGroup("01_advance.02_mount", "Mount");
    ins->addConfig("01_advance.02_mount.00_auto_mount",
                   { { "key", "00_auto_mount" },
                     { "text", "Auto mount" },
                     { "type", "mountCheckBox" },
                     { "default", true } });
    ins->addConfig("01_advance.02_mount.01_auto_mount_and_open",
                   { { "key", "01_auto_mount_and_open" },
                     { "text", "Open after auto mount" },
                     { "type", "openCheckBox" },
                     { "default", false } });
    ins->addCheckBoxConfig("01_advance.02_mount.02_mtp_show_bottom_info",
                           "Show item counts and sizes in the path of mounted MTP devices",
                           false);
    ins->addConfig("01_advance.02_mount.04_merge_the_entries_of_samba_shared_folders",
                   { { "key", "04_merge_the_entries_of_samba_shared_folders" },
                     { "text", "Merge the entries of Samba shared folders" },
                     { "type", "checkBoxWithMessage" },
                     { "message", "Switching the entry display may lead to failed mounting" },
                     { "default", true } });

    ins->addGroup("01_advance.03_dialog", "Dialog");
    ins->addCheckBoxConfig("01_advance.03_dialog.00_default_chooser_dialog",
                           "Use the file chooser dialog of File Manager");
    ins->addCheckBoxConfig("01_advance.03_dialog.01_delete_confirmation_dialog",
                           "Ask for my confirmation when deleting files",
                           false);

    ins->addGroup("01_advance.05_other", "Other");
    ins->addCheckBoxConfig("01_advance.05_other.00_hide_builtin_partition",
                           "Hide built-in disks on the Computer page",
                           false);
    //    ins->addCheckBoxConfig("01_advance.05_other.01_show_crumbbar_clickable_area",
    //                           "Show crumb bar clickable area");
    ins->addCheckBoxConfig("01_advance.05_other.02_hide_loop_partitions",
                           "Hide loop partitions on the Computer page");
    ins->addCheckBoxConfig("01_advance.05_other.03_show_filesystemtag_on_diskicon",
                           "Show file system on disk icon",
                           false);
    //    ins->addCheckBoxConfig("01_advance.05_other.04_extend_file_name",
    //                           "");
}

void SettingBackendPrivate::saveAsAppAttr(const QString &key, const QVariant &val)
{
    int attribute = keyToAA.value(key, static_cast<Application::ApplicationAttribute>(-1));
    if (attribute >= 0)
        Application::instance()->setAppAttribute(static_cast<Application::ApplicationAttribute>(attribute), val);
}

QVariant SettingBackendPrivate::getAsAppAttr(const QString &key)
{
    int attribute = keyToAA.value(key, static_cast<Application::ApplicationAttribute>(-1));
    if (attribute >= 0)
        return Application::instance()->appAttribute(static_cast<Application::ApplicationAttribute>(attribute));
    return QVariant();
}

void SettingBackendPrivate::saveAsGenAttr(const QString &key, const QVariant &val)
{
    int attribute = keyToGA.value(key, static_cast<Application::GenericAttribute>(-1));
    if (attribute >= 0)
        Application::instance()->setGenericAttribute(static_cast<Application::GenericAttribute>(attribute), val);
}

QVariant SettingBackendPrivate::getAsGenAttr(const QString &key)
{
    int attribute = keyToGA.value(key, static_cast<Application::GenericAttribute>(-1));
    if (attribute >= 0)
        return Application::instance()->genericAttribute(static_cast<Application::GenericAttribute>(attribute));
    return QVariant();
}

void SettingBackendPrivate::saveByFunc(const QString &key, const QVariant &val)
{
    if (setters.contains(key)) {
        auto setter = setters.value(key);
        if (setter)
            setter(val);
    }
}

QVariant SettingBackendPrivate::getByFunc(const QString &key)
{
    if (getters.contains(key)) {
        auto getter = getters.value(key);
        if (getter)
            return getter();
    }
    return QVariant();
}
