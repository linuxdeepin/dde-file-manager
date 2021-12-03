/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef DFMSETTINGDIALOG_P_H
#define DFMSETTINGDIALOG_P_H

#include "dfm-base/base/application/application.h"

#include <dsettingsbackend.h>

#include <QList>
#include <QVariant>

DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

template<typename Key, typename T>
class BidirectionHash
{
public:
#ifdef Q_COMPILER_INITIALIZER_LISTS
    inline BidirectionHash(std::initializer_list<std::pair<Key, T>> list)
    {
        k2v.reserve(int(list.size()));
        v2k.reserve(int(list.size()));

        for (auto it = list.begin(); it != list.end(); ++it) {
            k2v.insert(it->first, it->second);
            v2k.insert(it->second, it->first);
        }
    }
#endif

    bool containsKey(const Key &key) const
    {
        return k2v.contains(key);
    }
    bool containsValue(const T &value) const
    {
        return v2k.contains(value);
    }
    const Key key(const T &value) const
    {
        return v2k.value(value);
    }
    const Key key(const T &value, const Key &defaultKey) const
    {
        return v2k.value(value, defaultKey);
    }
    const T value(const Key &key) const
    {
        return k2v.value(key);
    }
    const T value(const Key &key, const T &defaultValue) const
    {
        return k2v.value(key, defaultValue);
    }
    QList<Key> keys() const
    {
        return k2v.keys();
    }

private:
    QHash<Key, T> k2v;
    QHash<T, Key> v2k;
};

class SettingBackend : public DSettingsBackend
{
    Q_OBJECT
public:
    explicit SettingBackend(QObject *parent = nullptr);

    QStringList keys() const;
    QVariant getOption(const QString &key) const;

    void doSync();

protected:
    void doSetOption(const QString &key, const QVariant &value);
    void onValueChanged(int attribute, const QVariant &value);

private:
    BidirectionHash<QString, Application::ApplicationAttribute> keyToAA {
        { "base.open_action.allways_open_on_new_window", Application::kAllwayOpenOnNewWindow },
        { "base.open_action.open_file_action", Application::kOpenFileMode },
        { "base.new_tab_windows.default_window_path", Application::kUrlOfNewWindow },
        { "base.new_tab_windows.new_tab_path", Application::kUrlOfNewTab },
        { "base.default_view.icon_size", Application::kIconSizeLevel },
        { "base.default_view.view_mode", Application::kViewMode },
        { "base.default_view.view_size_adjustable", Application::kViewSizeAdjustable },
    };
    BidirectionHash<QString, Application::GenericAttribute> keyToGA {
        { "base.hidden_files.show_hidden", Application::kShowedHiddenFiles },
        { "base.hidden_files.hide_suffix", Application::kShowedFileSuffixOnRename },
        { "base.hidden_files.show_recent", Application::kShowRecentFileEntry },
        { "advance.index.index_internal", Application::kIndexInternal },
        { "advance.index.index_external", Application::kIndexExternal },
        { "advance.index.index_search", Application::kIndexFullTextSearch },
        { "advance.search.show_hidden", Application::kShowedHiddenOnSearch },
        { "advance.preview.compress_file_preview", Application::kPreviewCompressFile },
        { "advance.preview.text_file_preview", Application::kPreviewTextFile },
        { "advance.preview.document_file_preview", Application::kPreviewDocumentFile },
        { "advance.preview.image_file_preview", Application::kPreviewImage },
        { "advance.preview.video_file_preview", Application::kPreviewVideo },
        { "advance.mount.auto_mount", Application::kAutoMount },
        { "advance.mount.auto_mount_and_open", Application::kAutoMountAndOpen },
        { "advance.mount.mtp_show_bottom_info", Application::kMTPShowBottomInfo },
        { "advance.mount.always_show_offline_remote_connection", Application::kAlwaysShowOfflineRemoteConnections },
        { "advance.dialog.default_chooser_dialog", Application::kOverrideFileChooserDialog },
        { "advance.dialog.delete_confirmation_dialog", Application::kShowDeleteConfirmDialog },
        { "advance.other.hide_system_partition", Application::kHiddenSystemPartition },
        { "advance.other.show_crumbbar_clickable_area", Application::kShowCsdCrumbBarClickableArea },
        { "advance.other.show_filesystemtag_on_diskicon", Application::kShowFileSystemTagOnDiskIcon },
    };
};

#endif   // DFMSETTINGDIALOG_P_H
