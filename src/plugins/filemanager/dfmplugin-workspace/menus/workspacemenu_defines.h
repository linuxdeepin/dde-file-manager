/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#ifndef WORKSPACEMENU_DEFINES_H
#define WORKSPACEMENU_DEFINES_H

namespace dfmplugin_workspace {

namespace ActionID {
inline constexpr char kSortBy[] = "sort-by";
inline constexpr char kDisplayAs[] = "display-as";
inline constexpr char kRefresh[] = "refresh";

// sort by
inline constexpr char kSrtName[] = "sort-by-name";
inline constexpr char kSrtTimeModified[] = "sort-by-time-modified";
inline constexpr char kSrtSize[] = "sort-by-size";
inline constexpr char kSrtType[] = "sort-by-type";

// display by
inline constexpr char kDisplayIcon[] = "display-as-icon";
inline constexpr char kDisplayList[] = "display-as-list";
}

}

#endif   // WORKSPACEMENU_DEFINES_H
