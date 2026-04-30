// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "signalhandler.h"

#include <QHash>
#include <QSet>
#include <QSocketNotifier>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace dfmbase {

// ---------------------------------------------------------------------------
// Pimpl: hides socketpair internals and Qt dependencies from the header
// ---------------------------------------------------------------------------

class SignalHandler::Private
{
public:
    explicit Private(SignalHandler *qq);
    ~Private();

    /*!
     * \brief Lazily create the socketpair and QSocketNotifier on first use.
     *
     * Defers resource allocation from the constructor to the first watchSignal()
     * call, ensuring the notifier exists iff signals are actually registered.
     * This avoids the state inconsistency where the constructor's socketpair
     * fails but a later watchSignal() retry succeeds with no notifier to read.
     *
     * Socket FD flags:
     *   - FD_CLOEXEC: Prevents FD leakage into child processes after fork/exec.
     *   - O_NONBLOCK:  Ensures write() in the signal handler never blocks.
     *                  Ensures read() in handleSignal() returns EAGAIN when drained.
     *
     * \return true if the notifier is ready, false if socketpair creation failed.
     */
    bool ensureSignalNotifier();

    bool watchSignal(int signalToWatch);
    bool ignoreSignal(int signalToIgnore);

    /*!
     * \brief Drain all pending signals from the socketpair and emit them.
     *
     * Called by QSocketNotifier::activated when data is available on signalFd[1].
     * The loop reads until EAGAIN/EWOULDBLOCK, ensuring coalesced signals from
     * rapid delivery are all processed in a single event loop iteration.
     *
     * The notifier is disabled before the loop to prevent re-entrant activation
     * (Qt may re-check the FD if its activated signal triggers further activity).
     * It is re-enabled after the loop completes.
     */
    void handleSignal();

    /*!
     * \brief Restore all signal dispositions to their pre-registration state.
     *
     * Called during destruction to undo all sigaction changes made by
     * watchSignal() and ignoreSignal(). Uses the saved previousActions map.
     */
    void restoreSignals();

    /*!
     * \brief Static POSIX signal handler — the ONLY code that runs in signal context.
     *
     * \async-signal-safety
     * This function must ONLY call async-signal-safe operations:
     *   - errno save/restore (write() may clobber errno).
     *   - write() to the socketpair (async-signal-safe on Linux).
     *
     * It intentionally does NOT:
     *   - Call qApp->quit() or any Qt function (not async-signal-safe).
     *   - Use logging macros (not async-signal-safe).
     *   - Allocate memory (malloc is not async-signal-safe).
     *
     * The signal number (int) is written atomically to signalFd[0].
     * On Linux, Unix domain SOCK_STREAM sockets guarantee atomicity
     * for writes up to PIPE_BUF (4096 bytes on x86-64), far exceeding
     * sizeof(int).
     */
    static void signalHandler(int signal);

    /*!
     * \brief The two ends of the Unix domain socketpair.
     *
     * Declared static so the POSIX signal handler (a plain C function with
     * no `this` pointer) can access it without going through the object.
     *
     *   signalFd[0] — write end: the signal handler writes the signal number here.
     *   signalFd[1] — read end:  QSocketNotifier watches this for readable data.
     *
     * Initialized to {-1, -1} and created on first use by ensureSignalNotifier().
     */
    static int signalFd[2];

    SignalHandler *q { nullptr };
    QSocketNotifier *notifier { nullptr };

    /// Signals currently registered for delivery via signalReceived().
    QSet<int> watchedSignals;

    /// Signals currently set to SIG_IGN via ignoreSignal().
    QSet<int> ignoredSignals;

    /// Saved previous sigaction for each modified signal, used by restoreSignals().
    /// Only the first previous action per signal is saved (before any modification).
    QHash<int, struct sigaction> previousActions;
};

int SignalHandler::Private::signalFd[2] { -1, -1 };

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

/*!
 * Meyers singleton — thread-safe in C++11+ (guaranteed by the standard).
 * Construction happens on first call, which must be from the main thread
 * since QSocketNotifier requires a running event loop in the creating thread.
 */
SignalHandler *SignalHandler::instance()
{
    static SignalHandler handler;
    return &handler;
}

SignalHandler::SignalHandler(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

SignalHandler::~SignalHandler()
{
    delete d;
}

bool SignalHandler::watchSignal(int signalToWatch)
{
    return d->watchSignal(signalToWatch);
}

bool SignalHandler::ignoreSignal(int signalToIgnore)
{
    return d->ignoreSignal(signalToIgnore);
}

// ---------------------------------------------------------------------------
// Private implementation
// ---------------------------------------------------------------------------

SignalHandler::Private::Private(SignalHandler *qq)
    : q(qq)
{
    // Intentionally empty — socketpair and notifier are created lazily
    // by ensureSignalNotifier() on the first watchSignal() call.
}

SignalHandler::Private::~Private()
{
    restoreSignals();

    delete notifier;
    notifier = nullptr;

    // Close socketpair FDs. Setting to -1 prevents the static signalHandler()
    // from attempting to write to closed FDs if a signal arrives during
    // the brief window between close() and program exit.
    if (signalFd[0] != -1) {
        ::close(signalFd[0]);
        signalFd[0] = -1;
    }

    if (signalFd[1] != -1) {
        ::close(signalFd[1]);
        signalFd[1] = -1;
    }
}

bool SignalHandler::Private::ensureSignalNotifier()
{
    if (notifier)
        return true;

    // Create the socketpair. SOCK_STREAM (not SOCK_DGRAM) is used so that
    // write(signalFd[0], &int, sizeof(int)) is atomic and message-boundary
    // preserving on Unix domain sockets — the read side always gets complete
    // int values, never partial reads.
    if (signalFd[0] == -1 || signalFd[1] == -1) {
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, signalFd) != 0) {
            qCWarning(logDFMBase) << "SignalHandler: Failed to create socketpair:" << ::strerror(errno);
            return false;
        }

        for (int fd : signalFd) {
            // FD_CLOEXEC: close-on-exec prevents child processes from inheriting
            // these FDs after fork/exec (e.g., when launching external processes).
            const int fdFlags = ::fcntl(fd, F_GETFD, 0);
            if (fdFlags != -1)
                ::fcntl(fd, F_SETFD, fdFlags | FD_CLOEXEC);

            // O_NONBLOCK: critical for correctness:
            //   - Write side: ensures signalHandler() never blocks (the kernel
            //     buffer for Unix domain sockets is at least 4KB, enough for many
            //     signal numbers; if full, write() returns EAGAIN which we ignore).
            //   - Read side: allows handleSignal() to drain all pending signals
            //     in a loop, detecting "no more data" via EAGAIN.
            const int statusFlags = ::fcntl(fd, F_GETFL, 0);
            if (statusFlags != -1)
                ::fcntl(fd, F_SETFL, statusFlags | O_NONBLOCK);
        }
    }

    // Watch the read end (signalFd[1]) for incoming signal data.
    // QSocketNotifier emits activated(int) when the FD becomes readable.
    notifier = new QSocketNotifier(signalFd[1], QSocketNotifier::Read, q);
    QObject::connect(notifier, &QSocketNotifier::activated, q, [this] {
        handleSignal();
    });

    return true;
}

bool SignalHandler::Private::watchSignal(int signalToWatch)
{
    if (!ensureSignalNotifier())
        return false;

    // Idempotent: if already watching this signal, no-op.
    if (watchedSignals.contains(signalToWatch))
        return true;

    // Install our signal handler via sigaction (preferred over signal() because
    // it provides SA_RESTART and reliable previous-action retrieval).
    struct sigaction action;
    ::memset(&action, 0, sizeof(action));
    action.sa_handler = SignalHandler::Private::signalHandler;
    ::sigemptyset(&action.sa_mask);
    // SA_RESTART: automatically restart interrupted system calls (read, write,
    // accept, etc.) so that signal delivery doesn't cause spurious EINTR errors
    // throughout the application.
    action.sa_flags = SA_RESTART;

    struct sigaction previousAction;
    ::memset(&previousAction, 0, sizeof(previousAction));
    if (::sigaction(signalToWatch, &action, &previousAction) != 0) {
        qCWarning(logDFMBase) << "SignalHandler: Failed to watch signal" << signalToWatch << ":" << ::strerror(errno);
        return false;
    }

    // Save the original disposition only once (the first time this signal is
    // modified), so restoreSignals() can undo all changes in reverse order.
    if (!previousActions.contains(signalToWatch))
        previousActions.insert(signalToWatch, previousAction);
    watchedSignals.insert(signalToWatch);
    ignoredSignals.remove(signalToWatch);
    return true;
}

bool SignalHandler::Private::ignoreSignal(int signalToIgnore)
{
    // Idempotent: if already ignoring this signal, no-op.
    if (ignoredSignals.contains(signalToIgnore))
        return true;

    struct sigaction action;
    ::memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN;
    ::sigemptyset(&action.sa_mask);

    struct sigaction previousAction;
    ::memset(&previousAction, 0, sizeof(previousAction));
    if (::sigaction(signalToIgnore, &action, &previousAction) != 0) {
        qCWarning(logDFMBase) << "SignalHandler: Failed to ignore signal" << signalToIgnore << ":" << ::strerror(errno);
        return false;
    }

    if (!previousActions.contains(signalToIgnore))
        previousActions.insert(signalToIgnore, previousAction);
    ignoredSignals.insert(signalToIgnore);
    watchedSignals.remove(signalToIgnore);
    return true;
}

void SignalHandler::Private::restoreSignals()
{
    // Restore each signal to its disposition before we modified it.
    // This is called during destruction to leave the process in a clean state
    // for any code that runs after the SignalHandler singleton is destroyed.
    const auto signals = previousActions.keys();
    for (int signal : signals) {
        const struct sigaction previousAction = previousActions.value(signal);
        ::sigaction(signal, &previousAction, nullptr);
    }

    previousActions.clear();
    watchedSignals.clear();
    ignoredSignals.clear();
}

void SignalHandler::Private::signalHandler(int signal)
{
    // ── CRITICAL: This function runs in asynchronous signal context. ──
    // Only async-signal-safe operations are permitted here.
    // See: man 7 signal-safety

    const int savedErrno = errno;

    if (signalFd[0] != -1) {
        // write() to a Unix domain SOCK_STREAM socket is async-signal-safe
        // and atomic for messages up to PIPE_BUF bytes (sizeof(int) = 4).
        // The return value is ignored: if the kernel buffer is full, the
        // signal is silently dropped. This is acceptable because SIGTERM
        // is typically delivered once, and the process exits on first receipt.
        const ssize_t ret = ::write(signalFd[0], &signal, sizeof(signal));
        (void)ret;
    }

    errno = savedErrno;
}

void SignalHandler::Private::handleSignal()
{
    // Disable the notifier before reading to prevent re-entrant activation.
    // Qt may immediately re-check the FD if a signal handler fires during
    // signalReceived() emission, causing a stack overflow without this guard.
    if (notifier)
        notifier->setEnabled(false);

    int signal = 0;
    while (true) {
        const ssize_t ret = ::read(signalFd[1], &signal, sizeof(signal));

        if (ret == sizeof(signal)) {
            // Successfully read a complete signal number — deliver it.
            // signalReceived() is emitted from the Qt event loop (main thread),
            // so connected slots can safely call any Qt or application code.
            Q_EMIT q->signalReceived(signal);
            continue;
        }

        if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;  // All pending signals drained — socket buffer is empty.

        if (ret != 0)
            qCWarning(logDFMBase) << "SignalHandler: Failed to read signal:" << ::strerror(errno);
        break;  // Unexpected read size or error — stop draining.
    }

    // Re-enable the notifier for the next signal arrival.
    if (notifier)
        notifier->setEnabled(true);
}

}   // namespace dfmbase
