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

// view defines
const QList<int> kIconSizeList { 48, 64, 96, 128, 256 };

// tab defines
const int kMaxTabCount = 8;

DPWORKSPACE_END_NAMESPACE

#endif   // DFMPLUGIN_WORKSPACE_GLOBAL_H
