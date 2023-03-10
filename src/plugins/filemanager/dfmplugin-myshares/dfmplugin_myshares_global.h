// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_MYSHARES_GLOBAL_H
#define DFMPLUGIN_MYSHARES_GLOBAL_H

#define DPMYSHARES_NAMESPACE dfmplugin_myshares
#define DPMYSHARES_BEGIN_NAMESPACE namespace DPMYSHARES_NAMESPACE {
#define DPMYSHARES_END_NAMESPACE }
#define DPMYSHARES_USE_NAMESPACE using namespace DPMYSHARES_NAMESPACE;

#include <QVariantMap>

DPMYSHARES_BEGIN_NAMESPACE

namespace ShareInfoKeys {
inline constexpr char kName[] { "shareName" };
inline constexpr char kPath[] { "path" };
inline constexpr char kComment[] { "comment" };
inline constexpr char kAcl[] { "acl" };
inline constexpr char kGuestEnable[] { "guestEnable" };
inline constexpr char kWritable[] { "writable" };
}

namespace MySharesActionId {
inline constexpr char kOpenShareFolder[] { "open-share-folder" };
inline constexpr char kOpenShareInNewWin[] { "open-share-in-new-win" };
inline constexpr char kOpenShareInNewTab[] { "open-share-in-new-tab" };
inline constexpr char kCancleSharing[] { "cancel-sharing" };
inline constexpr char kShareProperty[] { "share-property" };
}

namespace AcName {
inline constexpr char kAcSidebarShareMenu[] { "user_share_menu" };
}

typedef QVariantMap ShareInfo;
typedef QList<QVariantMap> ShareInfoList;

DPMYSHARES_END_NAMESPACE

#endif   // DFMPLUGIN_MYSHARES_GLOBAL_H
