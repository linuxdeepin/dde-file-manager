// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_UTILS_GLOBAL_H
#define DFMPLUGIN_UTILS_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPUTILS_NAMESPACE dfmplugin_utils

#define DPUTILS_BEGIN_NAMESPACE namespace DPUTILS_NAMESPACE {
#define DPUTILS_END_NAMESPACE }
#define DPUTILS_USE_NAMESPACE using namespace DPUTILS_NAMESPACE;

DPUTILS_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPUTILS_NAMESPACE)
DPUTILS_END_NAMESPACE

#endif   // DFMPLUGIN_UTILS_GLOBAL_H
