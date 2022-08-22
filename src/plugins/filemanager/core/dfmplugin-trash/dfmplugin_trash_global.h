/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
