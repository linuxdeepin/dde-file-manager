// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_AVFSBROWSER_GLOBAL_H
#define DFMPLUGIN_AVFSBROWSER_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPAVFSBROWSER_NAMESPACE dfmplugin_avfsbrowser

#define DPAVFSBROWSER_BEGIN_NAMESPACE namespace DPAVFSBROWSER_NAMESPACE {
#define DPAVFSBROWSER_END_NAMESPACE }
#define DPAVFSBROWSER_USE_NAMESPACE using namespace DPAVFSBROWSER_NAMESPACE;

DPAVFSBROWSER_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPAVFSBROWSER_NAMESPACE)

namespace AvfsMenuActionId {
inline constexpr char kOpen[] { "act-avfs-open" };
inline constexpr char kCopy[] { "act-avfs-copy" };
inline constexpr char kProperty[] { "act-avfs-property" };
}

DPAVFSBROWSER_END_NAMESPACE

#endif   // DFMPLUGIN_AVFSBROWSER_GLOBAL_H
