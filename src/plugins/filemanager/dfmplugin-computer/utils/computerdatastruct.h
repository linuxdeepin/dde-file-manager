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
DPCOMPUTER_BEGIN_NAMESPACE
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
    QString groupName;
    int groupId;
    QWidget *widget { nullptr };
    bool isEditing = false;
    DFMEntryFileInfoPointer info { nullptr };
};

namespace SuffixInfo {
static constexpr char kAppEntry[] { "appentry" };
static constexpr char kBlock[] { "blockdev" };
static constexpr char kProtocol[] { "protodev" };
static constexpr char kStashedProtocol[] { "protodevstashed" };
static constexpr char kUserDir[] { "userdir" };
}

namespace DeviceId {
static constexpr char kBlockDeviceIdPrefix[] { "/org/freedesktop/UDisks2/block_devices/" };
}

namespace ContextMenuActionTrs {
extern QString trOpenInNewWin();
extern QString trOpenInNewTab();
extern QString trMount();
extern QString trUnmount();
extern QString trRename();
extern QString trFormat();
extern QString trErase();
extern QString trEject();
extern QString trLogoutAndClearSavedPasswd();
extern QString trRemove();
extern QString trSafelyRemove();
extern QString trProperties();
extern QString trOpen();
}   // namespace ContextMenuActionTrs

DPCOMPUTER_END_NAMESPACE
#endif   // COMPUTERDATASTRUCT_H
