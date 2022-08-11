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

namespace ddplugin_organizer {

namespace ActionID {
inline constexpr char kOrganizeDesktop[] = "organize-desktop";
inline constexpr char kOrganizeOptions[] = "organize-options";
inline constexpr char kOrganizeBy[] = "organize-by";
inline constexpr char kOrganizeByType[] = "organize-by-type";
inline constexpr char kOrganizeByTimeAccessed[] = "organize-by-time-accessed";
inline constexpr char kOrganizeByTimeModified[] = "organize-by-time-modified";
inline constexpr char kOrganizeByTimeCreated[] = "organize-by-time-created";
inline constexpr char kOrganizeByCustom[] = "custom-collection";
inline constexpr char kCreateACollection[] = "create-a-collection";
inline constexpr char kDisplaySize[] = "display-size";
inline constexpr char kDisplaySizeSmaller[] = "display-size-smaller";
inline constexpr char kDisplaySizeNormal[] = "display-size-normal";
inline constexpr char kDisplaySizeLarger[] = "display-size-larger";
} // namespace ActionID

namespace CollectionMenuParams {
inline constexpr char kOnColletion[] = "OnColletion";
}
}

#endif // ORGANIZERMENU_DEFINES_H
