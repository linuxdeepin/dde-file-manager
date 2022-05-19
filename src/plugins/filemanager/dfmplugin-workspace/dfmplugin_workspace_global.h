/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMPLUGIN_WORKSPACE_GLOBAL_H
#define DFMPLUGIN_WORKSPACE_GLOBAL_H

#define DPWORKSPACE_BEGIN_NAMESPACE namespace dfmplugin_workspace {
#define DPWORKSPACE_END_NAMESPACE }
#define DPWORKSPACE_USE_NAMESPACE using namespace dfmplugin_workspace;
#define DPWORKSPACE_NAMESPACE dfmplugin_workspace

#include <QList>

DPWORKSPACE_BEGIN_NAMESPACE

enum class DirOpenMode : uint8_t {
    kOpenInCurrentWindow,
    kOpenNewWindow,
    //kForceOpenNewWindow // Todo(yanghao): ???
};

// view defines
inline QList<int> iconSizeList()
{
    return { 48, 64, 96, 128, 256 };
}

inline constexpr int kIconViewSpacing { 5 };
inline constexpr int kListViewSpacing { 0 };
inline constexpr int kIconModeColumnPadding { 10 };
inline constexpr int kDefualtHeaderSectionWidth { 140 };
inline constexpr int kMinimumHeaderSectionWidth { 120 };
inline constexpr int kListViewIconSize { 24 };
inline constexpr int kLeftPadding { 10 };
inline constexpr int kRightPadding { 10 };
inline constexpr int kListModeLeftMargin { 10 };
inline constexpr int kListModeRightMargin { 10 };
inline constexpr int kColumnPadding { 10 };

// tab defines
inline constexpr int kMaxTabCount { 8 };

DPWORKSPACE_END_NAMESPACE

#endif   // DFMPLUGIN_WORKSPACE_GLOBAL_H
