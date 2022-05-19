/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef DFMPLUGIN_MYSHARES_GLOBAL_H
#define DFMPLUGIN_MYSHARES_GLOBAL_H

#define DPMYSHARES_NAMESPACE dfmplugin_myshares
#define DPMYSHARES_BEGIN_NAMESPACE namespace DPMYSHARES_NAMESPACE {
#define DPMYSHARES_END_NAMESPACE }
#define DPMYSHARES_USE_NAMESPACE using namespace DPMYSHARES_NAMESPACE;

DPMYSHARES_BEGIN_NAMESPACE

namespace MySharesActionId {
inline constexpr char kOpenShareFolder[] { "open-share-folder" };
inline constexpr char kOpenShareInNewWin[] { "open-share-in-new-win" };
inline constexpr char kOpenShareInNewTab[] { "open-share-in-new-tab" };
inline constexpr char kCancleSharing[] { "cancel-sharing" };
inline constexpr char kShareProperty[] { "share-property" };
}

DPMYSHARES_END_NAMESPACE

#endif   // DFMPLUGIN_MYSHARES_GLOBAL_H
