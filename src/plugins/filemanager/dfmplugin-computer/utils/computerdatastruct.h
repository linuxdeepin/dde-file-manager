/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef COMPUTERDATASTRUCT_H
#define COMPUTERDATASTRUCT_H

#include "dfmplugin_computer_global.h"

#include "dfm-base/file/entry/entryfileinfo.h"

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
    DFMEntryFileInfoPointer info { nullptr };
};

namespace SuffixInfo {
inline constexpr char kAppEntry[] { "appentry" };
inline constexpr char kBlock[] { "blockdev" };
inline constexpr char kProtocol[] { "protodev" };
inline constexpr char kStashedProtocol[] { "protodevstashed" };
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
inline constexpr char kRemove[] { "computer-remove" };
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
extern QString trRemove();
extern QString trEject();
extern QString trErase();
extern QString trSafelyRemove();
extern QString trLogoutAndClearSavedPasswd();
extern QString trProperties();
}   // namespace ContextMenuActionTrs

}
#endif   // COMPUTERDATASTRUCT_H
