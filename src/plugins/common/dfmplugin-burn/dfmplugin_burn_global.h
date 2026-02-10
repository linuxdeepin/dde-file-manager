// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_BURN_GLOBAL_H
#define DFMPLUGIN_BURN_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPBURN_NAMESPACE dfmplugin_burn

#define DPBURN_BEGIN_NAMESPACE namespace DPBURN_NAMESPACE {
#define DPBURN_END_NAMESPACE }
#define DPBURN_USE_NAMESPACE using namespace DPBURN_NAMESPACE;

DPBURN_BEGIN_NAMESPACE
namespace ActionId {
inline constexpr char kStageKey[] { "stage-file-to-burning" };
inline constexpr char kStagePrex[] { "_stage-file-to-burning-" };
inline constexpr char kSendToOptical[] { "send-file-to-burnning-" };
inline constexpr char kMountImageKey[] { "mount-image" };
}
DFM_LOG_USE_CATEGORY(DPBURN_NAMESPACE)
DPBURN_END_NAMESPACE

#endif   // DFMPLUGIN_BURN_GLOBAL_H
