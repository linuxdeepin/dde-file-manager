// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_SIDEBAR_GLOBAL_H
#define DFMPLUGIN_SIDEBAR_GLOBAL_H

#define DPSIDEBAR_NAMESPACE dfmplugin_sidebar

#define DPSIDEBAR_BEGIN_NAMESPACE namespace DPSIDEBAR_NAMESPACE {
#define DPSIDEBAR_END_NAMESPACE }
#define DPSIDEBAR_USE_NAMESPACE using namespace DPSIDEBAR_NAMESPACE;

#include <dfm-framework/event/eventhelper.h>

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
inline constexpr char kNotExistedGroup[] { "__not_existed_group" };
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
inline constexpr char kVisiableControlKey[] { "Property_Key_VisiableControl" };   // a string, used to identify the item when visiable state changed in dconfig;
inline constexpr char kReportName[] { "Property_Key_ReportName" };   // a string, used to report log
// calllbacks
inline constexpr char kCallbackItemClicked[] { "Property_Key_CallbackItemClicked" };   // value is ItemClickedActionCallback
inline constexpr char kCallbackContextMenu[] { "Property_Key_CallbackContextMenu" };   // value is ContextMenuCallback
inline constexpr char kCallbackRename[] { "Property_Key_CallbackRename" };   // value is RenameCallback
inline constexpr char kCallbackFindMe[] { "Property_Key_CallbackFindMe" };   // value is FindMeCallback

}   // namespace PropertyKey

namespace ConfigInfos {
inline constexpr char kConfName[] { "org.deepin.dde.file-manager.sidebar" };
inline constexpr char kVisiableKey[] { "itemVisiable" };
inline constexpr char kGroupExpandedKey[] { "groupExpanded" };
}   // namespace ConfigInfos

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
    QString visiableControlKey;
    QString reportName;

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
          visiableControlKey({ map[PropertyKey::kVisiableControlKey].toString() }),
          reportName({ map[PropertyKey::kReportName].toString() }),
          clickedCb { DPF_NAMESPACE::paramGenerator<ItemClickedActionCallback>(map[PropertyKey::kCallbackItemClicked]) },
          contextMenuCb { DPF_NAMESPACE::paramGenerator<ContextMenuCallback>(map[PropertyKey::kCallbackContextMenu]) },
          renameCb { DPF_NAMESPACE::paramGenerator<RenameCallback>(map[PropertyKey::kCallbackRename]) },
          findMeCb { DPF_NAMESPACE::paramGenerator<FindMeCallback>(map[PropertyKey::kCallbackFindMe]) }
    {
    }

    inline bool operator==(const ItemInfo &info) const
    {
        return (url.scheme() == info.url.scheme()
                && url.path() == info.url.path()
                && group == info.group
                && url.host() == info.url.host());
    }
};

namespace AcName {
inline constexpr char kAcDmSideBar[] { "left_side_bar" };
inline constexpr char kAcDmSideBarView[] { "side_bar_view" };
inline constexpr char kAcSidebarMenuDefault[] { "default_sidebar_menu" };
}

DPSIDEBAR_END_NAMESPACE

Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::ItemClickedActionCallback);
Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::ContextMenuCallback);
Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::RenameCallback);
Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::FindMeCallback);

#endif   // DFMPLUGIN_SIDEBAR_GLOBAL_H
