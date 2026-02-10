// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_MENU_GLOBAL_H
#define DFMPLUGIN_MENU_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPMENU_NAMESPACE dfmplugin_menu

#define DPMENU_BEGIN_NAMESPACE namespace DPMENU_NAMESPACE {
#define DPMENU_END_NAMESPACE }
#define DPMENU_USE_NAMESPACE using namespace DPMENU_NAMESPACE;

DPMENU_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPMENU_NAMESPACE)
DPMENU_END_NAMESPACE

#endif   // DFMPLUGIN_MENU_GLOBAL_H
