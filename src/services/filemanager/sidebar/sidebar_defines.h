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
#ifndef SIDEBAR_DEFINES_H
#define SIDEBAR_DEFINES_H

#include "dfm_filemanager_service_global.h"

#include <QObject>
#include <QUrl>

#include <functional>

DSB_FM_BEGIN_NAMESPACE

namespace SideBar {
namespace DefaultGroup {
extern const char *const kCommon;
extern const char *const kDevice;
extern const char *const kBookmark;
extern const char *const kNetwork;
extern const char *const kTag;
extern const char *const kOther;
}   // namespace DefaultGroup

// custom event type
namespace EventType {
extern const int kEjectAction;
};   // namespace EventType

using CdActionCallback = std::function<void(quint64 windowId, const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
using RenameCallback = std::function<void(quint64 windowId, const QUrl &url, const QString &name)>;
using FindMeCallback = std::function<bool(const QUrl &itemUrl, const QUrl &targetUrl)>;

struct ItemInfo
{
    QString group;   //< SideBar::DefaultGroup
    QString iconName;
    QString text;
    QUrl url;
    Qt::ItemFlags flags;   //< Qt::ItemFlag
    bool removable { false };   //< item will set actionList(right edge)

    CdActionCallback cdCb { nullptr };
    ContextMenuCallback contextMenuCb { nullptr };
    RenameCallback renameCb { nullptr };
    FindMeCallback findMeCb { nullptr };

    bool operator==(const ItemInfo &info) const
    {
        return (url.scheme() == info.url.scheme()
                && url.path() == info.url.path()
                && group == info.group);
    }
};

}   // namespace SideBar

DSB_FM_END_NAMESPACE
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::SideBar::ItemInfo)
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::SideBar::CdActionCallback)
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::SideBar::ContextMenuCallback)
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::SideBar::RenameCallback)
Q_DECLARE_METATYPE(DSB_FM_NAMESPACE::SideBar::FindMeCallback)

#endif   // SIDEBAR_DEFINES_H
