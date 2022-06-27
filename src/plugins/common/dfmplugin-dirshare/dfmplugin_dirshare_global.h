/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
