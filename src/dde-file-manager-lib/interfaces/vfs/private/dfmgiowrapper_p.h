/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#ifndef DFMGIOWRAPPER_P_H
#define DFMGIOWRAPPER_P_H

#undef signals
extern "C" {
    #include <gio/gio.h>
}
#define signals public

// this struct calls "g_object_unref" to `delete` the pointer
struct ScopedPointerGObjectUnrefDeleter
{
    static inline void cleanup(void* pointer) {
        if (pointer != nullptr) {
            g_object_unref(pointer);
        }
    }
};

// this struct calls "g_free" to free the pointer
struct ScopedPointerGFreeDeleter
{
    static inline void cleanup(void* pointer) {
        if (pointer != nullptr) {
            g_free(pointer);
        }
    }
};

// this struct calls "g_list_free_full" with "g_object_unref" to free the pointer
struct ScopedPointerGListGObjectUnrefDeleter
{
    static inline void cleanup(GList* pointer) {
        if (pointer != nullptr) {
            g_list_free_full(pointer, g_object_unref);
        }
    }
};

using DFMGFile = QScopedPointer<GFile, ScopedPointerGObjectUnrefDeleter>;
using DFMGMount = QScopedPointer<GMount, ScopedPointerGObjectUnrefDeleter>;
using DFMGIcon = QScopedPointer<GIcon, ScopedPointerGObjectUnrefDeleter>;
using DFMGCChar = QScopedPointer<char, ScopedPointerGFreeDeleter>;
using DFMGFileInfo = QScopedPointer<GFileInfo, ScopedPointerGObjectUnrefDeleter>;
using DFMGMountList = QScopedPointer<GList, ScopedPointerGListGObjectUnrefDeleter>;

#endif // DFMGIOWRAPPER_P_H
