// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_TEXTINDEX_GLOBAL_H
#define SERVICE_TEXTINDEX_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>
#include <dfm-search/dsearch_global.h>

#define SERVICETEXTINDEX_NAMESPACE service_textindex
#define SERVICETEXTINDEX_BEGIN_NAMESPACE namespace SERVICETEXTINDEX_NAMESPACE {
#define SERVICETEXTINDEX_END_NAMESPACE }
#define SERVICETEXTINDEX_USE_NAMESPACE using namespace SERVICETEXTINDEX_NAMESPACE;

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace Defines {
inline constexpr char kLastUpdateTime[] { "lastUpdateTime" };
}   // namespace Defines

DFM_LOG_USE_CATEGORY(SERVICETEXTINDEX_NAMESPACE)

SERVICETEXTINDEX_END_NAMESPACE

#endif   // SERVICE_TEXTINDEX_GLOBAL_H
