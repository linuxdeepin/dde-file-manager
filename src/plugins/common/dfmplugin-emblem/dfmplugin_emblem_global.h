// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_EMBLEM_GLOBAL_H
#define DFMPLUGIN_EMBLEM_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPEMBLEM_NAMESPACE dfmplugin_emblem

#define DPEMBLEM_BEGIN_NAMESPACE namespace DPEMBLEM_NAMESPACE {
#define DPEMBLEM_END_NAMESPACE }
#define DPEMBLEM_USE_NAMESPACE using namespace DPEMBLEM_NAMESPACE;

#include <QObject>

DPEMBLEM_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPEMBLEM_NAMESPACE)

enum class SystemEmblemType : uint8_t {
    kLink,
    kLock,
    kUnreadable,
    kShare
};

// view defines
const double kMinEmblemSize = 12.0;
const double kMaxEmblemSize = 128.0;

inline constexpr char kConfigPath[] { "org.deepin.dde.file-manager.emblem" };
inline constexpr char kHideSystemEmblems[] { "dfm.system.emblem.hidden" };

DPEMBLEM_END_NAMESPACE

#endif   // DFMPLUGIN_EMBLEM_GLOBAL_H
