// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/backtrace/backtrace.h>

#include <QCoreApplication>

#include <mutex>
#include <csignal>
#include <sstream>
#include <cstring>

#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>

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

// --- async-signal-safe helpers (anonymous namespace) ---

namespace {

// Fixed-size alternate stack: SIGSTKSZ is a runtime value on glibc 2.34+,
// so we use a compile-time constant instead.
constexpr int kAltStackSize = 64 * 1024;
alignas(64) char altStack[kAltStackSize];

// Application name cached at install time for async-signal-safe output.
char g_appName[256] = "";

// Compile-time signal name table — strsignal() is not async-signal-safe.
struct SigInfo {
    int signo;
    const char *name;
};

const SigInfo sigTable[] = {
    { SIGSEGV, "SIGSEGV" },
    { SIGABRT, "SIGABRT" },
    { SIGBUS,  "SIGBUS"  },
    { SIGILL,  "SIGILL"  },
    { SIGFPE,  "SIGFPE"  },
    { SIGINT,  "SIGINT"  },
};

const char *sigName(int sig)
{
    for (const auto &s : sigTable) {
        if (s.signo == sig)
            return s.name;
    }
    return "UNKNOWN";
}

// Write a null-terminated string to stderr — async-signal-safe.
// (simple loop instead of strlen, which may use global state)
void writeStr(const char *s)
{
    size_t len = 0;
    while (s[len])
        ++len;
    if (len > 0)
        write(STDERR_FILENO, s, len);
}

// Write a decimal integer to stderr — async-signal-safe.
void writeInt(int val)
{
    char buf[16];
    int pos = static_cast<int>(sizeof(buf)) - 1;
    buf[pos] = '\0';

    bool negative = val < 0;
    unsigned int uval = negative ? static_cast<unsigned int>(-val)
                                 : static_cast<unsigned int>(val);
    if (uval == 0) {
        buf[--pos] = '0';
    } else {
        while (uval > 0) {
            buf[--pos] = static_cast<char>('0' + (uval % 10));
            uval /= 10;
        }
    }
    if (negative)
        buf[--pos] = '-';

    write(STDERR_FILENO, buf + pos, sizeof(buf) - 1 - static_cast<size_t>(pos));
}

}   // anonymous namespace

/*!
 * \brief Async-signal-safe crash backtrace handler.
 *
 * Only calls async-signal-safe functions: write(), backtrace(),
 * backtrace_symbols_fd(), raise(). No heap allocation, no Qt
 * logging, no std::string / ostringstream / strsignal.
 *
 * \param sig signal number
 */
void stackTraceHandler(int sig)
{
    writeStr("****************** ");
    if (g_appName[0])
        writeStr(g_appName);
    else
        writeStr("application");
    writeStr(" crashed backtrace ******************\n");

    writeStr("Received signal ");
    writeInt(sig);
    writeStr(" (");
    writeStr(sigName(sig));
    writeStr(")\n");

    const int kMaxFrames = 100;
    void *frames[kMaxFrames];
    int numFrames = ::backtrace(frames, kMaxFrames);

    // skip the top three signal handler related frames
    const int kSkip = 3;
    if (numFrames > kSkip)
        backtrace_symbols_fd(frames + kSkip, numFrames - kSkip, STDERR_FILENO);

    writeStr("****************** end backtrace ******************\n");

#ifdef __SANITIZE_THREAD__
    writeStr(
        "==> NOTE: any above warnings about \"signal-unsafe call\" are\n"
        "==> ignorable, as they are expected when generating a stack\n"
        "==> trace because of a signal under TSAN.\n");
#endif

    // re-signal to default handler (so we still get core dump if needed...)
    // SA_RESETHAND was set in sigaction, so the handler is already reset.
    raise(sig);
}
}   // namespace inner
/*!
 * \brief installStackTraceHandler
 * Install the async-signal-safe crash backtrace handler.
 *
 * Sets up an alternate signal stack (sigaltstack) so the handler can
 * run even on stack overflow. Uses sigaction with SA_RESETHAND | SA_ONSTACK.
 * Pre-warms backtrace()/backtrace_symbols_fd() to trigger glibc's lazy
 * dlopen of libgcc_s, which may fail under heap corruption.
 */
void installStackTraceHandler()
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        // Cache application name for async-signal-safe output in handler
        if (qApp) {
            QByteArray name = qApp->applicationName().toLocal8Bit();
            size_t len = name.size();
            if (len >= sizeof(inner::g_appName))
                len = sizeof(inner::g_appName) - 1;
            memcpy(inner::g_appName, name.constData(), len);
            inner::g_appName[len] = '\0';
        }

        // Pre-warm backtrace()/backtrace_symbols_fd() — glibc lazily
        // dlopens libgcc_s on first call, which may fail under heap
        // corruption, resulting in zero frames.
        {
            void *dummy = nullptr;
            ::backtrace(&dummy, 1);
            int devNull = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
            if (devNull >= 0) {
                backtrace_symbols_fd(&dummy, 1, devNull);
                ::close(devNull);
            }
        }

        // Set up alternate signal stack for stack overflow protection
        stack_t ss = {};
        ss.ss_sp = inner::altStack;
        ss.ss_size = inner::kAltStackSize;
        ss.ss_flags = 0;
        sigaltstack(&ss, nullptr);

        // Install handler with sigaction — async-signal-safe registration
        struct sigaction sa = {};
        sa.sa_handler = inner::stackTraceHandler;
        sa.sa_flags = SA_RESETHAND | SA_ONSTACK;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGSEGV, &sa, nullptr);
#ifdef DPF_FULLSIG_STRACE_ENABLE
        sigaction(SIGABRT, &sa, nullptr);
        sigaction(SIGBUS, &sa, nullptr);
        sigaction(SIGINT, &sa, nullptr);
#endif
    });
}
}   // namespace backtrace
DPF_END_NAMESPACE
