// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_CORE_GLOBAL_H
#define DFMPLUGIN_CORE_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPCORE_NAMESPACE dfmplugin_core

#define DPCORE_BEGIN_NAMESPACE namespace DPCORE_NAMESPACE {
#define DPCORE_END_NAMESPACE }
#define DPCORE_USE_NAMESPACE using namespace DPCORE_NAMESPACE;

DPCORE_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPCORE_NAMESPACE)
DPCORE_END_NAMESPACE
#endif   // DFMPLUGIN_CORE_GLOBAL_H
