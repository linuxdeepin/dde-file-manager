// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <dfm-framework/dfm_framework_global.h>

DPF_BEGIN_NAMESPACE
namespace backtrace {
namespace inner {
std::string demangle(void *value);
void printStack(void *frames[], int numFrames);
void printStack(int firstFramesToSkip);
void stackTraceHandler(int sig);
}   // namespace inner
void installStackTraceHandler();
}   // namespace backtrace
DPF_END_NAMESPACE
#endif   // BACKTRACE_H
