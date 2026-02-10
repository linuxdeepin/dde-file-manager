// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/backtrace/backtrace.h>

#include <QCoreApplication>

#include <mutex>
#include <csignal>
#include <sstream>

#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

DPF_BEGIN_NAMESPACE
namespace backtrace {

namespace inner {
/*!
 * \brief demangle ABI-mandated entry point in the
 * C++ runtime library for demangling
 * \param value backtrace string
 * \return demangled value
 */
std::string demangle(void *value)
{
    if (!value) {
        qCDebug(logDPF) << "Demangle: null pointer provided";
        return "";
    }

    std::ostringstream ostream;
    ostream.imbue(std::locale::classic());
    ostream << value << " : ";
    Dl_info info = { nullptr, nullptr, nullptr, nullptr };
    if (dladdr(value, &info) == 0) {
        ostream << "???";
        qCDebug(logDPF) << "Demangle: failed to get symbol info for address" << value;
    } else {
        if (info.dli_sname) {
            int status = 0;
            char *demangledName = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
            if (demangledName) {
                ostream << demangledName;
                free(demangledName);
            } else {
                ostream << info.dli_sname;
                qCDebug(logDPF) << "Demangle: failed to demangle symbol" << info.dli_sname << "status:" << status;
            }
        } else {
            ostream << "???";
        }

        long offset = reinterpret_cast<char *>(value) - reinterpret_cast<char *>(info.dli_saddr);
        ostream << std::hex << " + 0x" << offset;

        if (info.dli_fname)
            ostream << " @ " << info.dli_fname;
    }
    return ostream.str();
}

void printStack(void *frames[], int numFrames)
{
    qCInfo(logDPF) << "Printing stack trace with" << numFrames << "frames";
    for (int i = 0; i < numFrames; ++i) {
        const std::string &stackInfo = demangle(frames[i]);
        qCCritical(logDPF, "* %d>  %s", i, stackInfo.data());
    }
}

void printStack(int firstFramesToSkip)
{
    const int kMaxFrames = 100;
    void *frames[kMaxFrames];

    auto numFrames = ::backtrace(frames, kMaxFrames);
    qCDebug(logDPF) << "Captured" << numFrames << "frames, skipping first" << firstFramesToSkip;
    printStack(&frames[firstFramesToSkip], numFrames - firstFramesToSkip);
}

/*!
 * \brief Install a signal handler to print callstack on the following signals:
 *  SIGILL SIGSEGV SIGBUS SIGABRT
 * \param sig
 */
void stackTraceHandler(int sig)
{
    // reset to default handler
    signal(sig, SIG_DFL);
    qCCritical(logDPF, "Received signal %d (%s)\n", sig, strsignal(sig));

    QString head;
    head = QString("****************** %0 crashed backtrace ******************")
                   .arg(qApp->applicationName());
    QString end { head.size(), '*' };
    qCCritical(logDPF, "%s", head.toStdString().data());

    // skip the top three signal handler related frames
    printStack(3);

    qCCritical(logDPF, "%s", end.toStdString().data());

    // Efforts to fix or suppress TSAN warnings "signal-unsafe call inside of
    // a signal" have failed, so just warn the user about them.
#ifdef __SANITIZE_THREAD__
    fprintf(stderr,
            "==> NOTE: any above warnings about \"signal-unsafe call\" are\n"
            "==> ignorable, as they are expected when generating a stack\n"
            "==> trace because of a signal under TSAN. Consider why the\n"
            "==> signal was generated to begin with, and the stack trace\n"
            "==> in the TSAN warning can be useful for that. (The stack\n"
            "==> trace printed by the signal handler is likely obscured\n"
            "==> by TSAN output.)\n");
#endif

    qCInfo(logDPF) << "Re-raising signal" << sig << "to default handler";
    // re-signal to default handler (so we still get core dump if needed...)
    raise(sig);
}
}   // namespace inner
/*!
 * \brief initbacktrace
 * Register signal handler.
 */
void installStackTraceHandler()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        qCInfo(logDPF) << "Installing stack trace signal handlers";
        // just use the plain old signal as it's simple and sufficient
        // for this use case
        signal(SIGSEGV, inner::stackTraceHandler);
        qCDebug(logDPF) << "Registered SIGSEGV handler";
#ifdef DPF_FULLSIG_STRACE_ENABLE
        signal(SIGINT, stackTraceHandler);
        signal(SIGBUS, stackTraceHandler);
        signal(SIGABRT, stackTraceHandler);
        qCDebug(logDPF) << "Registered additional signal handlers: SIGINT, SIGBUS, SIGABRT";
#endif
        qCInfo(logDPF) << "Stack trace signal handlers installation completed";
    });
}
}   // namespace backtrace
DPF_END_NAMESPACE
