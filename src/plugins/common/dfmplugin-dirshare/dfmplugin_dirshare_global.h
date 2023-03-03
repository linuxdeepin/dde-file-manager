// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_DIRSHARE_GLOBAL_H
#define DFMPLUGIN_DIRSHARE_GLOBAL_H

#define DPDIRSHARE_NAMESPACE dfmplugin_dirshare

#define DPDIRSHARE_BEGIN_NAMESPACE namespace DPDIRSHARE_NAMESPACE {
#define DPDIRSHARE_END_NAMESPACE }
#define DPDIRSHARE_USE_NAMESPACE using namespace DPDIRSHARE_NAMESPACE;

#include <QVariantMap>

DPDIRSHARE_BEGIN_NAMESPACE

namespace ShareActionId {
inline constexpr char kActAddShareKey[] { "add-share" };
inline constexpr char kActRemoveShareKey[] { "remove-share" };
}

typedef QVariantMap ShareInfo;
typedef QList<QVariantMap> ShareInfoList;

namespace ShareInfoKeys {
inline constexpr char kName[] { "shareName" };
inline constexpr char kPath[] { "path" };
inline constexpr char kComment[] { "comment" };
inline constexpr char kAcl[] { "acl" };
inline constexpr char kGuestEnable[] { "guestEnable" };
inline constexpr char kWritable[] { "writable" };
inline constexpr char kAnonymous[] { "anonymous" };
}

inline constexpr char kEventSpace[] { "dfmplugin_dirshare" };

DPDIRSHARE_END_NAMESPACE

#endif   // DFMPLUGIN_DIRSHARE_GLOBAL_H
