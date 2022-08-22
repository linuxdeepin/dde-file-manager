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
#ifndef DFMPLUGIN_BURN_GLOBAL_H
#define DFMPLUGIN_BURN_GLOBAL_H

#define DPBURN_NAMESPACE dfmplugin_burn

#define DPBURN_BEGIN_NAMESPACE namespace DPBURN_NAMESPACE {
#define DPBURN_END_NAMESPACE }
#define DPBURN_USE_NAMESPACE using namespace DPBURN_NAMESPACE;

DPBURN_BEGIN_NAMESPACE
namespace ActionId {
inline constexpr char kStageKey[] { "stage-file-to-burning" };
inline constexpr char kStagePrex[] { "_stage-file-to-burning-" };
inline constexpr char kSendToOptical[] { "send-file-to-burnning-" };
inline constexpr char kMountImageKey[] { "mount-image" };
}
DPBURN_END_NAMESPACE

#endif   // DFMPLUGIN_BURN_GLOBAL_H
