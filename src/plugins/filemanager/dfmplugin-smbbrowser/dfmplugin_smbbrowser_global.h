// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
inline constexpr char kOpenSmbInNewTab[] { "open-smb-in-new-tab" };
inline constexpr char kMountSmb[] { "mount-smb" };
inline constexpr char kUnmountSmb[] { "umount-smb" };
inline constexpr char kUnmountAllSmb[] { "umount-all-smb" };
inline constexpr char kLogoutAndUnmountAllSmb[] { "logout-umount-all-smb" };
inline constexpr char kProperties[] { "properties-smb" };
}

DPSMBBROWSER_END_NAMESPACE

QDebug operator<<(QDebug dbg, const DPSMBBROWSER_NAMESPACE::SmbShareNode &node);

typedef QList<DPSMBBROWSER_NAMESPACE::SmbShareNode> SmbShareNodes;

using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;

Q_DECLARE_METATYPE(ContextMenuCallback);
#endif   // DFMPLUGIN_SMBBROWSER_GLOBAL_H
