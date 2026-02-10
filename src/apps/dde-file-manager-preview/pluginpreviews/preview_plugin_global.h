// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEW_PLUGIN_GLOBAL_H
#define PREVIEW_PLUGIN_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define PREVIEW_NAMESPACE plugin_filepreview

#define PREVIEW_BEGIN_NAMESPACE namespace PREVIEW_NAMESPACE {
#define PREVIEW_END_NAMESPACE }
#define PREVIEW_USE_NAMESPACE using namespace PREVIEW_NAMESPACE;

PREVIEW_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(PREVIEW_NAMESPACE)
PREVIEW_END_NAMESPACE

#endif   // PREVIEW_PLUGIN_GLOBAL_H
