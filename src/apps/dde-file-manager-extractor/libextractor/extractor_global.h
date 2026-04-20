// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTRACTOR_GLOBAL_H
#define EXTRACTOR_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define EXTRACTOR_NAMESPACE dfm_extractor

#define EXTRACTOR_BEGIN_NAMESPACE namespace EXTRACTOR_NAMESPACE {
#define EXTRACTOR_END_NAMESPACE }
#define EXTRACTOR_USE_NAMESPACE using namespace EXTRACTOR_NAMESPACE;

EXTRACTOR_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(EXTRACTOR_NAMESPACE)
EXTRACTOR_END_NAMESPACE

#endif   // EXTRACTOR_GLOBAL_H
