// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_SEARCH_GLOBAL_H
#define DFMPLUGIN_SEARCH_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPSEARCH_NAMESPACE dfmplugin_search
#define DPSEARCH_BEGIN_NAMESPACE namespace DPSEARCH_NAMESPACE {
#define DPSEARCH_END_NAMESPACE }
#define DPSEARCH_USE_NAMESPACE using namespace DPSEARCH_NAMESPACE;

// When the version of glib is greater than or equal to 2.66,
// the header files of glib cannot be included in extern "C"
#define GLIB_VERSION_MIN_REQUIRED GLIB_VERSION_2_50

DPSEARCH_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPSEARCH_NAMESPACE)

#define SEARCH_SETTING_GROUP "10_advance.00_search"

namespace SearchActionId {
inline constexpr char kOpenFileLocation[] { "open-file-location" };
inline constexpr char kSrtPath[] { "sort-by-path" };
}

namespace CustomKey {
inline constexpr char kDisableSearch[] { "Property_Key_DisableSearch" };
inline constexpr char kRedirectedPath[] { "Property_Key_RedirectedPath" };
}

namespace SearchSettings {
inline constexpr char kGroupSearch[] { SEARCH_SETTING_GROUP };
inline constexpr char kIndexInternal[] { SEARCH_SETTING_GROUP ".00_index_internal" };
inline constexpr char kIndexExternal[] { SEARCH_SETTING_GROUP ".01_index_external" };
inline constexpr char kFulltextSearch[] { SEARCH_SETTING_GROUP ".02_fulltext_search" };
}

namespace DConfig {
inline constexpr char kSearchCfgPath[] { "org.deepin.dde.file-manager.search" };
inline constexpr char kEnableFullTextSearch[] { "enableFullTextSearch" };
}

DPSEARCH_END_NAMESPACE

#endif   // DFMPLUGIN_SEARCH_GLOBAL_H
