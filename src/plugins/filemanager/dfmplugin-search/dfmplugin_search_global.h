// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_SEARCH_GLOBAL_H
#define DFMPLUGIN_SEARCH_GLOBAL_H

#define DPSEARCH_BEGIN_NAMESPACE namespace dfmplugin_search {
#define DPSEARCH_END_NAMESPACE }
#define DPSEARCH_USE_NAMESPACE using namespace dfmplugin_search;
#define DPSEARCH_NAMESPACE dfmplugin_search

// When the version of glib is greater than or equal to 2.66,
// the header files of glib cannot be included in extern "C"
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_50

DPSEARCH_BEGIN_NAMESPACE
namespace SearchActionId {
inline constexpr char kOpenFileLocation[] { "open-file-location" };
inline constexpr char kSrtPath[] { "sort-by-path" };
}

namespace CustomKey {
inline constexpr char kDisableSearch[] { "Property_Key_DisableSearch" };
inline constexpr char kRedirectedPath[] { "Property_Key_RedirectedPath" };
}
DPSEARCH_END_NAMESPACE

#endif   // DFMPLUGIN_SEARCH_GLOBAL_H
