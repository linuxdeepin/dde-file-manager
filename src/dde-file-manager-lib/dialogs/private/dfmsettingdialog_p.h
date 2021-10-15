/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include <QList>
#include <QVariant>
#include <dsettingsbackend.h>
#include "dfmapplication.h"

DCORE_USE_NAMESPACE
DFM_USE_NAMESPACE

template<typename Key, typename T>
class BidirectionHash
{
public:
#ifdef Q_COMPILER_INITIALIZER_LISTS
    inline BidirectionHash(std::initializer_list<std::pair<Key, T> > list)
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
    BidirectionHash<QString, DFMApplication::ApplicationAttribute> keyToAA {
        {"base.open_action.allways_open_on_new_window", DFMApplication::AA_AllwayOpenOnNewWindow},
        {"base.open_action.open_file_action", DFMApplication::AA_OpenFileMode},
        {"base.new_tab_windows.default_window_path", DFMApplication::AA_UrlOfNewWindow},
        {"base.new_tab_windows.new_tab_path", DFMApplication::AA_UrlOfNewTab},
        {"base.default_view.icon_size", DFMApplication::AA_IconSizeLevel},
        {"base.default_view.view_mode", DFMApplication::AA_ViewMode},
        {"base.default_view.view_size_adjustable", DFMApplication::AA_ViewSizeAdjustable},
    };
    BidirectionHash<QString, DFMApplication::GenericAttribute> keyToGA {
        {"base.hidden_files.show_hidden", DFMApplication::GA_ShowedHiddenFiles},
        {"base.hidden_files.hide_suffix", DFMApplication::GA_ShowedFileSuffixOnRename},
        {"base.hidden_files.show_recent", DFMApplication::GA_ShowRecentFileEntry},
        {"advance.index.index_internal", DFMApplication::GA_IndexInternal},
        {"advance.index.index_external", DFMApplication::GA_IndexExternal},
#ifdef FULLTEXTSEARCH_ENABLE
        {"advance.index.index_search", DFMApplication::GA_IndexFullTextSearch},
#endif
        {"advance.search.show_hidden", DFMApplication::GA_ShowedHiddenOnSearch},
        {"advance.preview.compress_file_preview", DFMApplication::GA_PreviewCompressFile},
        {"advance.preview.text_file_preview", DFMApplication::GA_PreviewTextFile},
        {"advance.preview.document_file_preview", DFMApplication::GA_PreviewDocumentFile},
        {"advance.preview.image_file_preview", DFMApplication::GA_PreviewImage},
        {"advance.preview.video_file_preview", DFMApplication::GA_PreviewVideo},
        {"advance.mount.auto_mount", DFMApplication::GA_AutoMount},
        {"advance.mount.auto_mount_and_open", DFMApplication::GA_AutoMountAndOpen},
        {"advance.mount.mtp_show_bottom_info", DFMApplication::GA_MTPShowBottomInfo},
        {"advance.mount.always_show_offline_remote_connection", DFMApplication::GA_AlwaysShowOfflineRemoteConnections},
        {"advance.dialog.default_chooser_dialog", DFMApplication::GA_OverrideFileChooserDialog},
        {"advance.dialog.delete_confirmation_dialog", DFMApplication::GA_ShowDeleteConfirmDialog},
        {"advance.other.hide_system_partition", DFMApplication::GA_HiddenSystemPartition},
        {"advance.other.show_crumbbar_clickable_area", DFMApplication::GA_ShowCsdCrumbBarClickableArea},
        {"advance.other.show_filesystemtag_on_diskicon", DFMApplication::GA_ShowFileSystemTagOnDiskIcon},
    };
};

#endif // DFMSETTINGDIALOG_P_H
