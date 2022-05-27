/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FILEDIALOGPLUGIN_CORE_GLOBAL_H
#define FILEDIALOGPLUGIN_CORE_GLOBAL_H

#define DIALOGCORE_NAMESPACE filedialog_core

#define DIALOGCORE_BEGIN_NAMESPACE namespace DIALOGCORE_NAMESPACE {
#define DIALOGCORE_END_NAMESPACE }
#define DIALOGCORE_USE_NAMESPACE using namespace DIALOGCORE_NAMESPACE;

DIALOGCORE_BEGIN_NAMESPACE

namespace UISize {
inline constexpr int kTitleMaxWidth { 200 };
}

DIALOGCORE_END_NAMESPACE

#endif   // FILEDIALOGPLUGIN_CORE_GLOBAL_H
