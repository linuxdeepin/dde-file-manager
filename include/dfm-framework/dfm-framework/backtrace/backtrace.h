// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <dfm-framework/dfm_framework_global.h>

DPF_BEGIN_NAMESPACE
namespace backtrace {

void installStackTraceHandler();

}
DPF_END_NAMESPACE
#endif   // BACKTRACE_H
