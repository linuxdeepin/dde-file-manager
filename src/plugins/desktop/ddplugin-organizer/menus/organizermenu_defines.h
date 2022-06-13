/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef ORGANIZERMENU_DEFINES_H
#define ORGANIZERMENU_DEFINES_H

#include "ddplugin_organizer_global.h"

DDP_ORGANIZER_BEGIN_NAMESPACE

namespace ActionID {
inline constexpr char kOrganizeDesktop[] = "organize-desktop";
inline constexpr char kOrganizeOptions[] = "organize-options";
inline constexpr char kOrganizeBy[] = "organize-by";
inline constexpr char kOrganizeByType[] = "organize-by-type";
inline constexpr char kOrganizeByTimeAccessed[] = "organize-by-time-accessed";
inline constexpr char kOrganizeByTimeModified[] = "organize-by-time-modified";
inline constexpr char kOrganizeByTimeCreated[] = "organize-by-time-created";
inline constexpr char kOrganizeByCustom[] = "custom-collection";

} // namespace ActionID

DDP_ORGANIZER_END_NAMESPACE

#endif // ORGANIZERMENU_DEFINES_H
