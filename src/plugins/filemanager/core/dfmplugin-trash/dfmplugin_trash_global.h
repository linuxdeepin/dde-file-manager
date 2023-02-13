// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_TRASH_GLOBAL_H
#define DFMPLUGIN_TRASH_GLOBAL_H

#define DPTRASH_NAMESPACE dfmplugin_trash

#define DPTRASH_BEGIN_NAMESPACE namespace dfmplugin_trash {
#define DPTRASH_END_NAMESPACE }
#define DPTRASH_USE_NAMESPACE using namespace dfmplugin_trash;

DPTRASH_BEGIN_NAMESPACE
namespace TrashActionId {
inline constexpr char kRestore[] { "restore" };
inline constexpr char kRestoreAll[] { "restore-all" };
inline constexpr char kEmptyTrash[] { "empty-trash" };
inline constexpr char kSourcePath[] { "sort-by-source-path" };
inline constexpr char kTimeDeleted[] { "sort-by-time-deleted" };
}
DPTRASH_END_NAMESPACE

#endif   // DFMPLUGIN_TRASH_GLOBAL_H
