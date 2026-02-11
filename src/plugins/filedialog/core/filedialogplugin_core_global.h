// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGPLUGIN_CORE_GLOBAL_H
#define FILEDIALOGPLUGIN_CORE_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DIALOGCORE_NAMESPACE filedialog_core

#define DIALOGCORE_BEGIN_NAMESPACE namespace DIALOGCORE_NAMESPACE {
#define DIALOGCORE_END_NAMESPACE }
#define DIALOGCORE_USE_NAMESPACE using namespace DIALOGCORE_NAMESPACE;

DIALOGCORE_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DIALOGCORE_NAMESPACE)

namespace UISize {
inline constexpr int kTitleMaxWidth { 200 };
}

namespace AcName {
inline constexpr char kAcFDStautsBar[] { "dlg_status_bar" };
inline constexpr char kAcFDStatusBarFileNameEdit[] { "file_name_edit" };
inline constexpr char kAcFDStatusBarFilters[] { "filters" };
inline constexpr char kAcFDStatusBarTitleLabel[] { "title_label" };
inline constexpr char kAcFDStatusBarContentLabel[] { "content_label" };
inline constexpr char kAcFDStatusBarContentEdit[] { "content_edit" };
inline constexpr char kAcFDStatusBarContentBox[] { "content_box" };
}

DIALOGCORE_END_NAMESPACE

#endif   // FILEDIALOGPLUGIN_CORE_GLOBAL_H
