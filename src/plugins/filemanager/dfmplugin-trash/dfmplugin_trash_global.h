// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_TRASH_GLOBAL_H
#define DFMPLUGIN_TRASH_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPTRASH_NAMESPACE dfmplugin_trash

#define DPTRASH_BEGIN_NAMESPACE namespace DPTRASH_NAMESPACE {
#define DPTRASH_END_NAMESPACE }
#define DPTRASH_USE_NAMESPACE using namespace DPTRASH_NAMESPACE;

DPTRASH_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPTRASH_NAMESPACE)

namespace TrashActionId {
inline constexpr char kRestore[] { "restore" };
inline constexpr char kRestoreAll[] { "restore-all" };
inline constexpr char kEmptyTrash[] { "empty-trash" };
inline constexpr char kSourcePath[] { "sort-by-source-path" };
inline constexpr char kTimeDeleted[] { "sort-by-time-deleted" };
// group by actions
inline constexpr char kGroupBySourcePath[] { "group-by-source-path" };
inline constexpr char kGroupByTimeDeleted[] { "group-by-time-deleted" };
}
DPTRASH_END_NAMESPACE

#endif   // DFMPLUGIN_TRASH_GLOBAL_H
