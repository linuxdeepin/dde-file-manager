// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
