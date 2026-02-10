// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDPLUGIN_CORE_GLOBAL_H
#define DDPLUGIN_CORE_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DDPCORE_NAMESPACE ddplugin_core

#define DDPCORE_BEGIN_NAMESPACE namespace DDPCORE_NAMESPACE {
#define DDPCORE_END_NAMESPACE }
#define DDPCORE_USE_NAMESPACE using namespace DDPCORE_NAMESPACE;

DDPCORE_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DDPCORE_NAMESPACE)
DDPCORE_END_NAMESPACE

#endif   // DDPLUGIN_CORE_GLOBAL_H
