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

#define TOP_GROUP_BASE "00_base"
#define LV2_GROUP_OPEN_ACTION "00_base.00_open_action"
#define LV2_GROUP_NEW_TAB_WINDOWS "00_base.01_new_tab_windows"
#define LV2_GROUP_FILES_AND_FOLDERS "00_base.02_files_and_folders"

#define TOP_GROUP_WORKSPACE "02_workspace"
#define LV2_GROUP_VIEW "02_workspace.00_view"
#define LV2_GROUP_PREVIEW "02_workspace.01_thumb_preview"
#define LV2_GROUP_COMPUTER_VIEW "02_workspace.02_computer"

#define TOP_GROUP_ADVANCE "10_advance"
#define LV2_GROUP_SEARCH "10_advance.00_search"
#define LV2_GROUP_MOUNT "10_advance.01_mount"
#define LV2_GROUP_DIALOG "10_advance.02_dialog"

BidirectionHash<QString, Application::ApplicationAttribute> SettingBackendPrivate::keyToAA {
    { LV2_GROUP_OPEN_ACTION ".00_allways_open_on_new_window", Application::kAllwayOpenOnNewWindow },
    { LV2_GROUP_OPEN_ACTION ".01_open_file_action", Application::kOpenFileMode },
    { LV2_GROUP_NEW_TAB_WINDOWS ".00_default_window_path", Application::kUrlOfNewWindow },
    { LV2_GROUP_NEW_TAB_WINDOWS ".01_new_tab_path", Application::kUrlOfNewTab },
    { LV2_GROUP_VIEW ".00_icon_size", Application::kIconSizeLevel },
    { LV2_GROUP_VIEW ".01_view_mode", Application::kViewMode },
    { LV2_GROUP_FILES_AND_FOLDERS ".02_mixed_sort", Application::kFileAndDirMixedSort },
};

BidirectionHash<QString, Application::GenericAttribute> SettingBackendPrivate::keyToGA {
    { LV2_GROUP_FILES_AND_FOLDERS ".00_show_hidden", Application::kShowedHiddenFiles },
    { LV2_GROUP_FILES_AND_FOLDERS ".01_show_suffix", Application::kShowedFileSuffix },
    { LV2_GROUP_SEARCH ".00_index_internal", Application::kIndexInternal },
    { LV2_GROUP_SEARCH ".01_index_external", Application::kIndexExternal },
    { LV2_GROUP_SEARCH ".02_index_search", Application::kIndexFullTextSearch },
    { LV2_GROUP_PREVIEW ".00_compress_file_preview", Application::kPreviewCompressFile },
    { LV2_GROUP_PREVIEW ".01_text_file_preview", Application::kPreviewTextFile },
    { LV2_GROUP_PREVIEW ".02_document_file_preview", Application::kPreviewDocumentFile },
    { LV2_GROUP_PREVIEW ".03_image_file_preview", Application::kPreviewImage },
    { LV2_GROUP_PREVIEW ".04_video_file_preview", Application::kPreviewVideo },
    { LV2_GROUP_PREVIEW ".05_audio_file_preview", Application::kPreviewAudio },
    { LV2_GROUP_PREVIEW ".06_remote_env_file_preview", Application::kShowThunmbnailInRemote },
    { LV2_GROUP_MOUNT ".00_auto_mount", Application::kAutoMount },
    { LV2_GROUP_MOUNT ".01_auto_mount_and_open", Application::kAutoMountAndOpen },
    { LV2_GROUP_MOUNT ".02_mtp_show_bottom_info", Application::kMTPShowBottomInfo },
    { LV2_GROUP_MOUNT ".04_merge_the_entries_of_samba_shared_folders", Application::kMergeTheEntriesOfSambaSharedFolders },
    { LV2_GROUP_DIALOG ".00_default_chooser_dialog", Application::kOverrideFileChooserDialog },
    { LV2_GROUP_DIALOG ".01_delete_confirmation_dialog", Application::kShowDeleteConfirmDialog },
    { LV2_GROUP_COMPUTER_VIEW ".01_hide_builtin_partition", Application::kHiddenSystemPartition },
    { LV2_GROUP_COMPUTER_VIEW ".02_hide_loop_partitions", Application::kHideLoopPartitions },
    { LV2_GROUP_COMPUTER_VIEW ".04_show_filesystemtag_on_diskicon", Application::kShowFileSystemTagOnDiskIcon },
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
    initBasicSettingConfig();
    initWorkspaceSettingConfig();
    initAdvanceSettingConfig();
}

void SettingBackend::initBasicSettingConfig()
{
    auto ins = SettingJsonGenerator::instance();

    // TODO(xust): these configs should be split into plugins.

    // base / open_behaviour
    ins->addGroup(TOP_GROUP_BASE, "Basic");
    ins->addGroup(LV2_GROUP_OPEN_ACTION, "Open behavior");
    ins->addCheckBoxConfig(LV2_GROUP_OPEN_ACTION ".00_allways_open_on_new_window",
                           "Always open folder in new window",
                           false);
    ins->addComboboxConfig(LV2_GROUP_OPEN_ACTION ".01_open_file_action",
                           "Open file:",
                           QStringList { "Click",
                                         "Double click" },
                           1);

    // base / new_win_and_tab
    ins->addGroup(LV2_GROUP_NEW_TAB_WINDOWS, "New window and tab");
    ins->addComboboxConfig(LV2_GROUP_NEW_TAB_WINDOWS ".00_default_window_path",
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
    ins->addComboboxConfig(LV2_GROUP_NEW_TAB_WINDOWS ".01_new_tab_path",
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

    // base / files_and_folders
    ins->addGroup(LV2_GROUP_FILES_AND_FOLDERS, tr("Files and folders"));
    ins->addCheckBoxConfig(LV2_GROUP_FILES_AND_FOLDERS ".00_show_hidden",
                           "Show hidden files",
                           false);
    ins->addCheckBoxConfig(LV2_GROUP_FILES_AND_FOLDERS ".01_show_suffix",
                           "Show file extensions");
    ins->addCheckBoxConfig(LV2_GROUP_FILES_AND_FOLDERS ".02_mixed_sort",
                           "Mix sorting of files and folders",
                           false);
}

void SettingBackend::initWorkspaceSettingConfig()
{
    auto ins = SettingJsonGenerator::instance();

    ins->addGroup(TOP_GROUP_WORKSPACE, tr("Workspace"));
    ins->addGroup(LV2_GROUP_VIEW, tr("View"));

    ins->addComboboxConfig(LV2_GROUP_VIEW ".00_icon_size",
                           "Default size:",
                           QStringList { "Extra small",
                                         "Small",
                                         "Medium",
                                         "Large",
                                         "Extra large" },
                           1);
    ins->addComboboxConfig(LV2_GROUP_VIEW ".01_view_mode",
                           "Default view:",
                           { { "values", QStringList { "Icon", "List" } },
                             { "keys", QVariantList { 1, 2 } } },
                           1);

    ins->addGroup(LV2_GROUP_PREVIEW, tr("Thumbnail preview"));

    ins->addCheckBoxConfig(LV2_GROUP_PREVIEW ".00_compress_file_preview",
                           "Compressed file preview",
                           false);
    ins->addCheckBoxConfig(LV2_GROUP_PREVIEW ".01_text_file_preview",
                           "Text preview");
    ins->addCheckBoxConfig(LV2_GROUP_PREVIEW ".02_document_file_preview",
                           "Document preview");
    ins->addCheckBoxConfig(LV2_GROUP_PREVIEW ".03_image_file_preview",
                           "Image preview");
    ins->addCheckBoxConfig(LV2_GROUP_PREVIEW ".04_video_file_preview",
                           "Video preview");
    ins->addCheckBoxConfig(LV2_GROUP_PREVIEW ".05_audio_file_preview",
                           "Music preview");
    ins->addConfig(LV2_GROUP_PREVIEW ".06_remote_env_file_preview",
                   { { "key", "06_remote_env_file_preview" },
                     { "text", "The remote environment shows thumbnail previews" },
                     { "message", "Turning on the thumbnail preview may cause the remote directory to load slowly or the operation to freeze" },
                     { "type", "checkBoxWithMessage" },
                     { "default", false } });
}

void SettingBackend::initAdvanceSettingConfig()
{
    auto ins = SettingJsonGenerator::instance();

    ins->addGroup(TOP_GROUP_ADVANCE, "Advanced");

    ins->addGroup(LV2_GROUP_SEARCH, tr("Search"));
    ins->addCheckBoxConfig(LV2_GROUP_SEARCH ".00_index_internal",
                           "Auto index internal disk");
    ins->addCheckBoxConfig(LV2_GROUP_SEARCH ".01_index_external",
                           "Index external storage device after connected to computer",
                           false);
    ins->addCheckBoxConfig(LV2_GROUP_SEARCH ".02_index_search",
                           "Full-Text search",
                           false);

    ins->addGroup(LV2_GROUP_MOUNT, "Mount");
    ins->addConfig(LV2_GROUP_MOUNT ".00_auto_mount",
                   { { "key", "00_auto_mount" },
                     { "text", "Auto mount" },
                     { "type", "mountCheckBox" },
                     { "default", true } });
    ins->addConfig(LV2_GROUP_MOUNT ".01_auto_mount_and_open",
                   { { "key", "01_auto_mount_and_open" },
                     { "text", "Open after auto mount" },
                     { "type", "openCheckBox" },
                     { "default", false } });
    ins->addCheckBoxConfig(LV2_GROUP_MOUNT ".02_mtp_show_bottom_info",
                           "Show item counts and sizes in the path of mounted MTP devices",
                           false);
    ins->addConfig(LV2_GROUP_MOUNT ".04_merge_the_entries_of_samba_shared_folders",
                   { { "key", "04_merge_the_entries_of_samba_shared_folders" },
                     { "text", "Merge the entries of Samba shared folders" },
                     { "type", "checkBoxWithMessage" },
                     { "message", "Switching the entry display may lead to failed mounting" },
                     { "default", true } });

    ins->addGroup(LV2_GROUP_DIALOG, "Dialog");
    ins->addCheckBoxConfig(LV2_GROUP_DIALOG ".00_default_chooser_dialog",
                           "Use the file chooser dialog of File Manager");
    ins->addCheckBoxConfig(LV2_GROUP_DIALOG ".01_delete_confirmation_dialog",
                           "Ask for my confirmation when deleting files",
                           false);
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
