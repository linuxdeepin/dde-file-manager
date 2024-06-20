// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H
#define APPLICATION_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

namespace dfmbase {

class Settings;
class ApplicationPrivate;
class Application : public QObject
{
    Q_OBJECT
    friend class ApplicationPrivate;

public:
    // Application-level configuration, default in the ~/.config/deepin/{AppName}/dde-file-manager.json
    enum ApplicationAttribute {
        kAllwayOpenOnNewWindow,
        kIconSizeLevel,
        kViewMode,
        kViewSizeAdjustable,   // adjustable table header width for list mode
        kViewComppactMode,
        kViewAutoCompace,
        kOpenFileMode,   // Click/double-click [0/1]
        kUrlOfNewWindow,   // default path to new window
        kUrlOfNewTab,   // default path to new tabs
        kThemeName,
        kFileAndDirMixedSort,   // Mixed sorting of ordinary files and directories
        kListItemExpandable   // Item expandable as tree in FileView with list view mode
    };

    Q_ENUM(ApplicationAttribute)

    // Generic configuration, default in the ~/.config/deepin/dde-file-manager.json
    enum GenericAttribute {
        kPreviewCompressFile,   // open the zip as a directory
        kPreviewTextFile,   // generate thumbnails from plain text
        kPreviewDocumentFile,   // document generation thumbnails (pdf)
        kPreviewImage,   // image generation thumbnails
        kPreviewVideo,   // video generation thumbnails
        kPreviewAudio,   // audio generation thumbnails
        kAutoMount,   // automatic mounting of hard disk devices
        kAutoMountAndOpen,   // automatically mount and open hard disk devices
        kAlwaysShowOfflineRemoteConnections,   // always show offline remote mounts (currently only smb mounts are resident)
        kMergeTheEntriesOfSambaSharedFolders,   // merge the entries of Samba shared folders
        kOverrideFileChooserDialog,   // dialog box when using DDE File Manager as an application to select files
        kShowedHiddenOnSearch,   // show hidden files when searching
        kShowedHiddenFiles,   // show hidden files
        kShowedFileSuffix,   // show suffix
        kDisableNonRemovableDeviceUnmount,   // disable local disk uninstallation
        kHiddenSystemPartition,   // hide system partition
        kShowCsdCrumbBarClickableArea,   // eave an area in the breadcrumb bar that can be clicked on to go to the edit state of the address bar
        kShowFileSystemTagOnDiskIcon,   // display file system information on the disk icon
        kShowDeleteConfirmDialog,   // display the delete confirmation dialog
        kHideLoopPartitions,   // hide loop partitions
        kShowThunmbnailInRemote,   // show file thumbnail in remote dir
    };

    Q_ENUM(GenericAttribute)

    enum TriggerAttribute {
        kRestoreViewMode,    // restore defualt view mode to all dir
        kClearSearchHistory     // clear search history
    };

    Q_ENUM(TriggerAttribute)

    explicit Application(QObject *parent = nullptr);
    ~Application();

    static QVariant appAttribute(ApplicationAttribute aa);
    static QUrl appUrlAttribute(ApplicationAttribute aa);
    static void setAppAttribute(ApplicationAttribute aa, const QVariant &value);
    static bool syncAppAttribute();

    static QVariant genericAttribute(GenericAttribute ga);
    static void setGenericAttribute(GenericAttribute ga, const QVariant &value);
    static bool syncGenericAttribute();

    static Application *instance();

    static Settings *genericSetting();
    static Settings *appSetting();

    static Settings *genericObtuselySetting();
    static Settings *appObtuselySetting();

    static Settings *dataPersistence();

    static void appAttributeTrigger(TriggerAttribute ta, quint64 winId);

Q_SIGNALS:
    void appAttributeChanged(ApplicationAttribute aa, const QVariant &value);
    void genericAttributeChanged(GenericAttribute ga, const QVariant &value);
    void appAttributeEdited(ApplicationAttribute aa, const QVariant &value);
    void genericAttributeEdited(GenericAttribute ga, const QVariant &value);
    void iconSizeLevelChanged(int level);
    void viewModeChanged(int mode);
    void previewCompressFileChanged(bool enable);
    void showedFileSuffixChanged(bool enable);
    void previewAttributeChanged(GenericAttribute ga, bool enable);
    void showedHiddenFilesChanged(bool enable);
    void csdClickableAreaAttributeChanged(bool enabled);
    void indexFullTextSearchChanged(bool enabled);

    void genericSettingCreated(Settings *settings);
    void appSettingCreated(Settings *settings);
    void clearSearchHistory(quint64 winId);

protected:
    Application(ApplicationPrivate *dd, QObject *parent = nullptr);

private:
    void onSettingsValueChanged(const QString &, const QString &, const QVariant &);
    void onSettingsValueEdited(const QString &, const QString &, const QVariant &);

    QScopedPointer<ApplicationPrivate> d;
};

}

#endif   // APPLICATION_H
