// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_RECENT_GLOBAL_H
#define DFMPLUGIN_RECENT_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPRECENT_NAMESPACE dfmplugin_recent

#define DPRECENT_BEGIN_NAMESPACE namespace DPRECENT_NAMESPACE {
#define DPRECENT_END_NAMESPACE }
#define DPRECENT_USE_NAMESPACE using namespace DPRECENT_NAMESPACE;

DPRECENT_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPRECENT_NAMESPACE)
namespace RecentActionID {
inline constexpr char kRemove[] { "remove" };
inline constexpr char kOpenFileLocation[] { "open-file-location" };
inline constexpr char kSortByPath[] { "sort-by-path" };
inline constexpr char kSortByLastRead[] { "sort-by-lastRead" };
// group by actions
inline constexpr char kGroupByPath[] { "group-by-path" };
inline constexpr char kGroupByLastRead[] { "group-by-lastRead" };
}
DPRECENT_END_NAMESPACE

#endif   // DFMPLUGIN_RECENT_GLOBAL_H
