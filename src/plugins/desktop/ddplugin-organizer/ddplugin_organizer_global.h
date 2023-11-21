// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDPLUGIN_ORGANIZER_GLOBAL_H
#define DDPLUGIN_ORGANIZER_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DDP_ORGANIZER_NAMESPACE ddplugin_organizer

#define DDP_ORGANIZER_BEGIN_NAMESPACE namespace DDP_ORGANIZER_NAMESPACE {
#define DDP_ORGANIZER_END_NAMESPACE }
#define DDP_ORGANIZER_USE_NAMESPACE using namespace DDP_ORGANIZER_NAMESPACE;

DDP_ORGANIZER_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DDP_ORGANIZER_NAMESPACE)
DDP_ORGANIZER_END_NAMESPACE

#endif   // DDPLUGIN_ORGANIZER_GLOBAL_H
