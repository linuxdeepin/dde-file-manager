// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>

namespace dfmbase {

/*!
 * \brief Thread-safe POSIX signal delivery bridge to the Qt event loop.
 *
 * \design
 * This class solves the fundamental incompatibility between POSIX signal handlers
 * (which can only call async-signal-safe functions) and the Qt event loop
 * (which requires non-trivial operations like QCoreApplication::quit()).
 *
 * It uses a variant of the classic "self-pipe trick":
 *   - A Unix domain socketpair (SOCK_STREAM) is created between two file descriptors.
 *   - The static signal handler writes the signal number (int) to signalFd[0].
 *   - A QSocketNotifier watches signalFd[1] from the Qt event loop.
 *   - When the notifier fires, the signal number is read and emitted as a Qt signal.
 *
 * \note socketpair (SOCK_STREAM) is used instead of pipe() because:
 *       1. It transmits the full int signal number atomically (not just 1 byte).
 *       2. SOCK_STREAM guarantees message boundaries for small writes on Unix
 *          domain sockets (atomic for up to PIPE_BUF bytes).
 *       3. It is bidirectional, providing clearer fd role separation:
 *          fd[0] = write end (signal handler), fd[1] = read end (Qt event loop).
 *
 * \concurrency
 * The singleton is accessed via Meyers singleton (static local). Construction
 * and first-use happen in the main thread before any signal registration, so
 * no additional synchronization is needed for the instance() call itself.
 *
 * \lifetime
 * As a Meyers singleton, destruction happens during program exit after the
 * Qt event loop has stopped. The destructor restores all previously saved
 * signal dispositions and closes the socketpair file descriptors.
 *
 * \usage
 * \code
 *   auto *handler = SignalHandler::instance();
 *   QObject::connect(handler, &SignalHandler::signalReceived, app, [](int sig) {
 *       if (sig == SIGTERM) QCoreApplication::quit();
 *   });
 *   handler->watchSignal(SIGTERM);
 *   handler->ignoreSignal(SIGPIPE);
 * \endcode
 */
class SignalHandler : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SignalHandler)

public:
    /*!
     * \brief Returns the singleton SignalHandler instance.
     * \note Thread-safe. First call must occur from the main thread.
     */
    static SignalHandler *instance();

    /*!
     * \brief Register a POSIX signal for delivery via the Qt event loop.
     *
     * Installs a sigaction handler that writes the signal number to the
     * internal socketpair. When the signal arrives, signalReceived(int)
     * is emitted from the main thread's event loop.
     *
     * \param signalToWatch  The POSIX signal number (e.g., SIGTERM).
     * \return true on success, false if socketpair creation or sigaction failed.
     *
     * \note This method lazily initializes the socketpair and QSocketNotifier
     *       on first successful call. If the socketpair cannot be created,
     *       all subsequent calls will also return false.
     * \note If the signal is already being watched, returns true immediately
     *       (idempotent). If it was previously ignored, it is now watched.
     */
    bool watchSignal(int signalToWatch);

    /*!
     * \brief Set a POSIX signal to be silently ignored.
     *
     * \param signalToIgnore  The POSIX signal number (e.g., SIGPIPE).
     * \return true on success, false if sigaction failed.
     *
     * \note If the signal is already ignored, returns true immediately.
     *       If it was previously watched, it is now ignored instead.
     */
    bool ignoreSignal(int signalToIgnore);

Q_SIGNALS:
    /*!
     * \brief Emitted when a watched POSIX signal is received.
     *
     * This signal is emitted from the Qt main thread's event loop,
     * making it safe to call any Qt or application code in connected slots.
     *
     * \param signal  The POSIX signal number that was received.
     */
    void signalReceived(int signal);

private:
    explicit SignalHandler(QObject *parent = nullptr);
    ~SignalHandler() override;

    class Private;
    Private *d;
};

}   // namespace dfmbase

#endif   // SIGNALHANDLER_H
