// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDPLUGIN_CANVAS_GLOBAL_H
#define DDPLUGIN_CANVAS_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DDP_CANVAS_NAMESPACE ddplugin_canvas

#define DDP_CANVAS_BEGIN_NAMESPACE namespace DDP_CANVAS_NAMESPACE {
#define DDP_CANVAS_END_NAMESPACE }
#define DDP_CANVAS_USE_NAMESPACE using namespace DDP_CANVAS_NAMESPACE;

DDP_CANVAS_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DDP_CANVAS_NAMESPACE)
DDP_CANVAS_END_NAMESPACE

#endif   // DDPLUGIN_CANVAS_GLOBAL_H
