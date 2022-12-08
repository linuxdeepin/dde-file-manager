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
#ifndef DFMPLUGIN_SMBBROWSER_GLOBAL_H
#define DFMPLUGIN_SMBBROWSER_GLOBAL_H

#include <QString>
#include <QDebug>

#define DPSMBBROWSER_NAMESPACE dfmplugin_smbbrowser
#define DPSMBBROWSER_BEGIN_NAMESPACE namespace DPSMBBROWSER_NAMESPACE {
#define DPSMBBROWSER_END_NAMESPACE }
#define DPSMBBROWSER_USE_NAMESPACE using namespace DPSMBBROWSER_NAMESPACE;

DPSMBBROWSER_BEGIN_NAMESPACE

struct SmbShareNode
{
    QString url;
    QString displayName;
    QString iconType;
};

namespace SmbBrowserActionId {
inline constexpr char kOpenSmb[] { "open-smb" };
inline constexpr char kOpenSmbInNewWin[] { "open-smb-in-new-win" };
inline constexpr char kMountSmb[] { "mount-smb" };
inline constexpr char kUnmountSmb[] { "umount-smb" };
inline constexpr char kUnmountAllSmb[] { "umount-all-smb" };
inline constexpr char kLogoutAndUnmountAllSmb[] { "logout-umount-all-smb" };
}

DPSMBBROWSER_END_NAMESPACE

QDebug operator<<(QDebug dbg, const DPSMBBROWSER_NAMESPACE::SmbShareNode &node);

typedef QList<DPSMBBROWSER_NAMESPACE::SmbShareNode> SmbShareNodes;

using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;

Q_DECLARE_METATYPE(ContextMenuCallback);
#endif   // DFMPLUGIN_SMBBROWSER_GLOBAL_H
