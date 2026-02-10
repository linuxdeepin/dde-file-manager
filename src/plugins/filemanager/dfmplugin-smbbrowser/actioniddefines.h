// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACTIONIDDEFINES_H
#define ACTIONIDDEFINES_H

#include "dfmplugin_smbbrowser_global.h"

DPSMBBROWSER_BEGIN_NAMESPACE

namespace SmbBrowserActionId {
inline constexpr char kOpenSmb[] { "open-smb" };
inline constexpr char kOpenSmbInNewWin[] { "open-smb-in-new-win" };
inline constexpr char kOpenSmbInNewTab[] { "open-smb-in-new-tab" };
inline constexpr char kMountSmb[] { "mount-smb" };
inline constexpr char kUnmountSmb[] { "umount-smb" };
inline constexpr char kProperties[] { "properties-smb" };
}

DPSMBBROWSER_END_NAMESPACE

#endif   // ACTIONIDDEFINES_H
