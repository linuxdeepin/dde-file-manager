/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef APPLICATION_H
#define APPLICATION_H

#include "dfm-base/dfm_base_global.h"

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
        kThemeName
    };

    Q_ENUM(ApplicationAttribute)

    // Generic configuration, default in the ~/.config/deepin/dde-file-manager.json
    enum GenericAttribute {
        kIndexInternal,
        kIndexExternal,
        kIndexFullTextSearch,   //full text search
        kPreviewCompressFile,   // open the zip as a directory
        kPreviewTextFile,   // generate thumbnails from plain text
        kPreviewDocumentFile,   // document generation thumbnails (pdf)
        kPreviewImage,   // image generation thumbnails
        kPreviewVideo,   // video generation thumbnails
        kAutoMount,   // automatic mounting of hard disk devices
        kAutoMountAndOpen,   // automatically mount and open hard disk devices
        kMTPShowBottomInfo,   // mtp show bottom statistics when mounted
        kAlwaysShowOfflineRemoteConnections,   // always show offline remote mounts (currently only smb mounts are resident)
        kOverrideFileChooserDialog,   // dialog box when using DDE File Manager as an application to select files
        kShowedHiddenOnSearch,   // show hidden files when searching
        kShowedHiddenFiles,   // show hidden files
        kShowedFileSuffix,   // show suffix
        kDisableNonRemovableDeviceUnmount,   // disable local disk uninstallation
        kHiddenSystemPartition,   // hide system partition
        kShowRecentFileEntry,   // show "Recent Documents" entry in the sidebar
        kShowCsdCrumbBarClickableArea,   // eave an area in the breadcrumb bar that can be clicked on to go to the edit state of the address bar
        kShowFileSystemTagOnDiskIcon,   // display file system information on the disk icon
        kShowDeleteConfirmDialog,   // display the delete confirmation dialog
        kHideLoopPartitions,   // hide loop partitions
    };

    Q_ENUM(GenericAttribute)

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
    void recentDisplayChanged(bool enable);
    void csdClickableAreaAttributeChanged(bool enabled);

    void genericSettingCreated(Settings *settings);
    void appSettingCreated(Settings *settings);

protected:
    Application(ApplicationPrivate *dd, QObject *parent = nullptr);

private:
    void onSettingsValueChanged(const QString &, const QString &, const QVariant &);
    void onSettingsValueEdited(const QString &, const QString &, const QVariant &);

    QScopedPointer<ApplicationPrivate> d;
};

}

#endif   // APPLICATION_H
