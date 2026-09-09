// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <dfm-framework/dfm_framework_global.h>

DPF_BEGIN_NAMESPACE
namespace backtrace {
namespace inner {

/*!
 * \brief Demangle a symbol address into a human-readable string.
 *
 * \note NOT async-signal-safe — uses ostringstream, dladdr, and
 * __cxa_demangle. Intended for non-signal debugging paths only.
 * The signal handler does NOT call this function.
 */
std::string demangle(void *value);

/*!
 * \brief Print a stack trace from a frame array.
 *
 * \note NOT async-signal-safe — calls demangle() internally.
 * Intended for non-signal debugging paths only.
 */
void printStack(void *frames[], int numFrames);

/*!
 * \brief Print a stack trace, skipping the first N frames.
 *
 * \note NOT async-signal-safe — calls demangle() internally.
 * Intended for non-signal debugging paths only.
 */
void printStack(int firstFramesToSkip);

/*!
 * \brief Async-signal-safe crash backtrace handler.
 *
 * Only calls async-signal-safe functions (write, backtrace,
 * backtrace_symbols_fd, raise). Installed via sigaction with
 * SA_RESETHAND | SA_ONSTACK on an alternate signal stack.
 */
void stackTraceHandler(int sig);
}   // namespace inner

/*!
 * \brief Install the async-signal-safe crash backtrace handler.
 *
 * Sets up an alternate signal stack (sigaltstack) and registers
 * the handler via sigaction. Pre-warms backtrace() and
 * backtrace_symbols_fd() to avoid glibc lazy dlopen under crash.
 * Safe to call multiple times (std::once_flag guarded).
 */
void installStackTraceHandler();
}   // namespace backtrace
DPF_END_NAMESPACE
#endif   // BACKTRACE_H
