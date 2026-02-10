// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_BOOKMARK_GLOBAL_H
#define DFMPLUGIN_BOOKMARK_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPBOOKMARK_NAMESPACE dfmplugin_bookmark

#define DPBOOKMARK_BEGIN_NAMESPACE namespace DPBOOKMARK_NAMESPACE {
#define DPBOOKMARK_END_NAMESPACE }
#define DPBOOKMARK_USE_NAMESPACE using namespace DPBOOKMARK_NAMESPACE;

DPBOOKMARK_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPBOOKMARK_NAMESPACE)

namespace BookmarkActionId {
inline constexpr char kActAddBookmarkKey[] { "add-bookmark" };
inline constexpr char kActRemoveBookmarkKey[] { "remove-bookmark" };
}

namespace AcName {
inline constexpr char kAcSidebarBookmarkMenu[] { "sidebar_bookmark_menu" };
}

DPBOOKMARK_END_NAMESPACE

#endif   // DFMPLUGIN_BOOKMARK_GLOBAL_H
