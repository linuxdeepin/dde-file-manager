/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DFMPLUGIN_SIDEBAR_GLOBAL_H
#define DFMPLUGIN_SIDEBAR_GLOBAL_H

#define DPSIDEBAR_NAMESPACE dfmplugin_sidebar

#define DPSIDEBAR_BEGIN_NAMESPACE namespace DPSIDEBAR_NAMESPACE {
#define DPSIDEBAR_END_NAMESPACE }
#define DPSIDEBAR_USE_NAMESPACE using namespace DPSIDEBAR_NAMESPACE;

#include <QUrl>
#include <QIcon>
#include <QString>
#include <QVariant>

#include <functional>

DPSIDEBAR_BEGIN_NAMESPACE

namespace DefaultGroup {
inline constexpr char kCommon[] { "Group_Common" };
inline constexpr char kDevice[] { "Group_Device" };
inline constexpr char kBookmark[] { "Group_Bookmark" };
inline constexpr char kNetwork[] { "Group_Network" };
inline constexpr char kTag[] { "Group_Tag" };
inline constexpr char kOther[] { "Group_Other" };
}   // namespace DefaultGroup

namespace PropertyKey {
inline constexpr char kUrl[] { "Property_Key_Url" };   // value is url, usually is empty
inline constexpr char kGroup[] { "Property_Key_Group" };   // value is string, belong to `DefaultGroup`
inline constexpr char kSubGroup[] { "Property_Key_SubGroup" };   // value is string, a custom group name used for sorting items
inline constexpr char kDisplayName[] { "Property_Key_DisplayName" };   // value is tring
inline constexpr char kIcon[] { "Property_Key_Icon" };   // value is QIcon
inline constexpr char kFinalUrl[] { "Property_Key_FinalUrl" };   // value is QUrl, this might be invalid, only for those items whose item url is different from real url
inline constexpr char kQtItemFlags[] { "Property_Key_QtItemFlags" };   // value is Qt::ItemFlag
inline constexpr char kIsEjectable[] { "Property_Key_Ejectable" };   // value is bool, tem will set actionList(right edge)
inline constexpr char kIsEditable[] { "Property_Key_Editable" };   // value is bool
inline constexpr char kIsHidden[] { "Property_Key_Hidden" };   // value is bool, item will be hidden
// calllbacks
inline constexpr char kCallbackItemClicked[] { "Property_Key_CallbackItemClicked" };   // value is ItemClickedActionCallback
inline constexpr char kCallbackContextMenu[] { "Property_Key_CallbackContextMenu" };   // value is ContextMenuCallback
inline constexpr char kCallbackRename[] { "Property_Key_CallbackRename" };   // value is RenameCallback
inline constexpr char kCallbackFindMe[] { "Property_Key_CallbackFindMe" };   // value is FindMeCallback

}   // namespace PropertyKey

using ItemClickedActionCallback = std::function<void(quint64 windowId, const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
using RenameCallback = std::function<void(quint64 windowId, const QUrl &url, const QString &name)>;
using FindMeCallback = std::function<bool(const QUrl &itemUrl, const QUrl &targetUrl)>;
using SortFunc = std::function<bool(const QUrl &, const QUrl &)>;

struct ItemInfo
{
    QUrl url;
    QString group;
    QString subGroup;
    QString displayName;
    QIcon icon;
    QUrl finalUrl;
    Qt::ItemFlags flags;
    bool isEjectable { false };   // TODO(zhangs): update
    bool isEditable { false };
    bool isHidden { false };   // TODO(zhangs): update

    ItemClickedActionCallback clickedCb { nullptr };
    ContextMenuCallback contextMenuCb { nullptr };
    RenameCallback renameCb { nullptr };
    FindMeCallback findMeCb { nullptr };

    ItemInfo() = default;
    inline ItemInfo(const QUrl &u, const QVariantMap &map)
        : url { u },
          group { map[PropertyKey::kGroup].toString() },
          subGroup { map[PropertyKey::kSubGroup].toString() },
          displayName { map[PropertyKey::kDisplayName].toString() },
          icon { qvariant_cast<QIcon>(map[PropertyKey::kIcon]) },
          finalUrl { map[PropertyKey::kFinalUrl].toUrl() },
          flags { qvariant_cast<Qt::ItemFlags>(map[PropertyKey::kQtItemFlags]) },
          isEjectable { map[PropertyKey::kIsEjectable].toBool() },
          clickedCb { qvariant_cast<ItemClickedActionCallback>(map[PropertyKey::kCallbackItemClicked]) },
          contextMenuCb { qvariant_cast<ContextMenuCallback>(map[PropertyKey::kCallbackContextMenu]) },
          renameCb { qvariant_cast<RenameCallback>(map[PropertyKey::kCallbackRename]) },
          findMeCb { qvariant_cast<FindMeCallback>(map[PropertyKey::kCallbackFindMe]) }
    {
    }

    inline bool operator==(const ItemInfo &info) const
    {
        return (url.scheme() == info.url.scheme()
                && url.path() == info.url.path()
                && group == info.group);
    }
};

DPSIDEBAR_END_NAMESPACE

Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::ItemClickedActionCallback);
Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::ContextMenuCallback);
Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::RenameCallback);
Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::FindMeCallback);

#endif   // DFMPLUGIN_SIDEBAR_GLOBAL_H
