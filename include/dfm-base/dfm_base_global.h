// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_BASE_GLOBAL_H
#define DFM_BASE_GLOBAL_H

#include <stdint.h>

#define DFMBASE_NAMESPACE dfmbase

#define DFMBASE_BEGIN_NAMESPACE namespace DFMBASE_NAMESPACE {
#define DFMBASE_END_NAMESPACE }
#define DFMBASE_USE_NAMESPACE using namespace DFMBASE_NAMESPACE;

#include <dfm-base/dfm_log_defines.h>

Q_DECLARE_LOGGING_CATEGORY(logDFMBase)

#endif   //DFM_BASE_GLOBAL_H
