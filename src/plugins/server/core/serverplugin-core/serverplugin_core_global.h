// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREPLUGIN_CORE_GLOBAL_H
#define COREPLUGIN_CORE_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define SERVERPCORE_NAMESPACE serverplugin_core

#define SERVERPCORE_BEGIN_NAMESPACE namespace SERVERPCORE_NAMESPACE {
#define SERVERPCORE_END_NAMESPACE }
#define SERVERPCORE_USE_NAMESPACE using namespace SERVERPCORE_NAMESPACE;

SERVERPCORE_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(SERVERPCORE_NAMESPACE)
SERVERPCORE_END_NAMESPACE

#endif   // SERVERPLUGIN_CORE_GLOBAL_H
