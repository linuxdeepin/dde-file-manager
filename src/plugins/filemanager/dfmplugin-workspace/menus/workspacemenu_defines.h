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

#include "dfmplugin_workspace_global.h"

DPWORKSPACE_BEGIN_NAMESPACE

namespace ActionID {
static constexpr char kSortBy[] = "sort-by";
static constexpr char kDisplayAs[] = "display-as";

// sort by
static constexpr char kSrtName[] = "sort-by-name";
static constexpr char kSrtTimeModified[] = "sort-by-time-modified";
static constexpr char kSrtSize[] = "sort-by-size";
static constexpr char kSrtType[] = "sort-by-type";

// display by
static constexpr char kDisplayIcon[] = "display-as-icon";
static constexpr char kDisplayList[] = "display-as-list";
}

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEMENU_DEFINES_H
