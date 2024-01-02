// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_SIDEBAR_GLOBAL_H
#define DFMPLUGIN_SIDEBAR_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>
#include <dfm-framework/event/eventhelper.h>

#include <QUrl>
#include <QIcon>
#include <QString>
#include <QVariant>

#include <functional>

#define DPSIDEBAR_NAMESPACE dfmplugin_sidebar

#define DPSIDEBAR_BEGIN_NAMESPACE namespace DPSIDEBAR_NAMESPACE {
#define DPSIDEBAR_END_NAMESPACE }
#define DPSIDEBAR_USE_NAMESPACE using namespace DPSIDEBAR_NAMESPACE;

DPSIDEBAR_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPSIDEBAR_NAMESPACE)

namespace DefaultGroup {
inline constexpr char kCommon[] { "Group_Common" };
inline constexpr char kDevice[] { "Group_Device" };
inline constexpr char kNetwork[] { "Group_Network" };
inline constexpr char kTag[] { "Group_Tag" };
inline constexpr char kOther[] { "Group_Other" };
inline constexpr char kNotExistedGroup[] { "__not_existed_group" };
}   // namespace DefaultGroup

namespace PropertyKey {
// value is url, usually is empty, used to update ItemInfo::url.
inline constexpr char kUrl[] { "Property_Key_Url" };

// value is string, belong to `DefaultGroup`
inline constexpr char kGroup[] { "Property_Key_Group" };

// value is string, a custom group name used for sorting items
inline constexpr char kSubGroup[] { "Property_Key_SubGroup" };

// value is string, as display
inline constexpr char kDisplayName[] { "Property_Key_DisplayName" };

// value is QIcon, as display
inline constexpr char kIcon[] { "Property_Key_Icon" };

// value is QUrl, this might be invalid, only for those items whose item url is different from real url
// Its business scenario is to provide a jump url for unmounted devices
inline constexpr char kFinalUrl[] { "Property_Key_FinalUrl" };

// value is Qt::ItemFlag
inline constexpr char kQtItemFlags[] { "Property_Key_QtItemFlags" };

// value is bool, tem will set actionList(right edge)
inline constexpr char kIsEjectable[] { "Property_Key_Ejectable" };

// value is bool
inline constexpr char kIsEditable[] { "Property_Key_Editable" };

// a string, used to identify the item when visiable state changed in dconfig
// If the value is "hidden_me", then the visibility control of the item will not be provided
inline constexpr char kVisiableControlKey[] { "Property_Key_VisiableControl" };

// a string, used to display in setting dialog.
// Usually, this property does not need to be set. It is only needed when multiple items share the same
// "Property_Key_VisibilityControl" (such as Tag).
inline constexpr char kVisiableDisplayName[] { "Property_Key_VisiableDisplayName" };

// a string, used to report log
inline constexpr char kReportName[] { "Property_Key_ReportName" };

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
    QString visiableControlKey;
    QString visiableDisplayName;
    QString reportName;

    ItemClickedActionCallback clickedCb { nullptr };
    ContextMenuCallback contextMenuCb { nullptr };
    RenameCallback renameCb { nullptr };
    FindMeCallback findMeCb { nullptr };

    ItemInfo() = default;
    inline ItemInfo(const QUrl &u, const QVariantMap &map = {})
        : url { u },
          group { map[PropertyKey::kGroup].toString() },
          subGroup { map[PropertyKey::kSubGroup].toString() },
          displayName { map[PropertyKey::kDisplayName].toString() },
          icon { qvariant_cast<QIcon>(map[PropertyKey::kIcon]) },
          finalUrl { map[PropertyKey::kFinalUrl].toUrl() },
          flags { qvariant_cast<Qt::ItemFlags>(map[PropertyKey::kQtItemFlags]) },
          isEjectable { map[PropertyKey::kIsEjectable].toBool() },
          visiableControlKey({ map[PropertyKey::kVisiableControlKey].toString() }),
          visiableDisplayName({ map[PropertyKey::kVisiableDisplayName].toString() }),
          reportName({ map[PropertyKey::kReportName].toString() }),
          clickedCb { DPF_NAMESPACE::paramGenerator<ItemClickedActionCallback>(map[PropertyKey::kCallbackItemClicked]) },
          contextMenuCb { DPF_NAMESPACE::paramGenerator<ContextMenuCallback>(map[PropertyKey::kCallbackContextMenu]) },
          renameCb { DPF_NAMESPACE::paramGenerator<RenameCallback>(map[PropertyKey::kCallbackRename]) },
          findMeCb { DPF_NAMESPACE::paramGenerator<FindMeCallback>(map[PropertyKey::kCallbackFindMe]) }
    {
        if (visiableControlKey.isEmpty())
            visiableControlKey = url.toString();
        if (visiableDisplayName.isEmpty())
            visiableDisplayName = displayName;
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
Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::ItemInfo);

#endif   // DFMPLUGIN_SIDEBAR_GLOBAL_H
