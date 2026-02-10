// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERDATASTRUCT_H
#define COMPUTERDATASTRUCT_H

#include "dfmplugin_computer_global.h"

#include <dfm-base/file/entry/entryfileinfo.h>

#include <QUrl>

class QWidget;
namespace dfmplugin_computer {
/*!
 * \brief The ComputerItemData struct
 * provide the basic info of computer item
 */
struct ComputerItemData
{
    /*!
     * \brief The ItemType enum
     * descripe the type of a item.
     */
    enum ShapeType {
        kSmallItem,
        kLargeItem,
        kSplitterItem,
        kWidgetItem,
    };

    QUrl url;   // entry://desktop.userdir
    ShapeType shape;
    mutable QString itemName;
    int groupId;
    QWidget *widget { nullptr };
    bool isEditing = false;
    bool isElided = false;
    bool isVisible = true;
    DFMEntryFileInfoPointer info { nullptr };
};

namespace SuffixInfo {
inline constexpr char kCommon[] { "_common_" };
inline constexpr char kAppEntry[] { "appentry" };
inline constexpr char kBlock[] { "blockdev" };
inline constexpr char kProtocol[] { "protodev" };
inline constexpr char kUserDir[] { "userdir" };
}

namespace DeviceId {
inline constexpr char kBlockDeviceIdPrefix[] { "/org/freedesktop/UDisks2/block_devices/" };
}

namespace ContextMenuAction {
inline constexpr char kOpen[] { "computer-open" };
inline constexpr char kOpenInNewWin[] { "computer-open-in-win" };
inline constexpr char kOpenInNewTab[] { "computer-open-in-tab" };
inline constexpr char kMount[] { "computer-mount" };
inline constexpr char kUnmount[] { "computer-unmount" };
inline constexpr char kRename[] { "computer-rename" };
inline constexpr char kFormat[] { "computer-format" };
inline constexpr char kEject[] { "computer-eject" };
inline constexpr char kErase[] { "computer-erase" };
inline constexpr char kSafelyRemove[] { "computer-safely-remove" };
inline constexpr char kLogoutAndForget[] { "computer-logout-and-forget-passwd" };
inline constexpr char kProperty[] { "computer-property" };

inline constexpr char kActionTriggeredFromSidebar[] { "trigger-from-sidebar" };

extern QString trOpen();
extern QString trOpenInNewWin();
extern QString trOpenInNewTab();
extern QString trMount();
extern QString trUnmount();
extern QString trRename();
extern QString trFormat();
extern QString trEject();
extern QString trErase();
extern QString trSafelyRemove();
extern QString trLogoutAndClearSavedPasswd();
extern QString trProperties();
}   // namespace ContextMenuActionTrs

}
#endif   // COMPUTERDATASTRUCT_H
