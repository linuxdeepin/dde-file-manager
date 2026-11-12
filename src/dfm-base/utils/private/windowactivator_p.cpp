// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowactivator_p.h"

#if defined(DFM_WITH_TREELAND_TOPLEVEL)

#    include <QCoreApplication>
#    include <QDebug>
#    include <QGuiApplication>
#    include <QStringList>
#    include <QWindow>

#    include <QtGui/qguiapplication_platform.h>
#    include <QtWaylandClient/QWaylandClientExtension>

#    include "qwayland-treeland-foreign-toplevel-manager-unstable-v2.h"

#    include <wayland-client-core.h>
#    include <wayland-util.h>

#    include <functional>
#    include <utility>

DFMBASE_BEGIN_NAMESPACE
namespace windowactivator {
namespace {

/*!
 * \brief One toplevel as described by the compositor
 *
 * Treeland reports every mapped toplevel of every client through this
 * protocol, so the fields needed to recognise one of our own windows - the
 * owning pid, the mirrored title and the minimized state - are cached here.
 */
class ForeignToplevelHandle : public QtWayland::treeland_foreign_toplevel_handle_v2
{
public:
    using ClosedHandler = std::function<void(ForeignToplevelHandle *)>;

    ForeignToplevelHandle(::treeland_foreign_toplevel_handle_v2 *object, ClosedHandler onClosed)
        : QtWayland::treeland_foreign_toplevel_handle_v2(object), m_onClosed(std::move(onClosed))
    {
    }

    ~ForeignToplevelHandle() override
    {
        if (isInitialized())
            destroy();
    }

    uint32_t pid() const { return m_pid; }
    const QString &title() const { return m_title; }
    bool isMinimized() const { return m_minimized; }

    void restoreAndActivate(struct ::wl_seat *seat)
    {
        if (m_minimized)
            unset_minimized();
        if (seat)
            activate(seat);
    }

protected:
    void treeland_foreign_toplevel_handle_v2_pid(uint32_t pid) override
    {
        m_pid = pid;
    }

    void treeland_foreign_toplevel_handle_v2_title(const QString &title) override
    {
        m_title = title;
    }

    void treeland_foreign_toplevel_handle_v2_state(struct ::wl_array *state) override
    {
        m_minimized = false;
        if (!state || !state->data)
            return;

        const auto *states = static_cast<const uint32_t *>(state->data);
        for (size_t i = 0; i < state->size / sizeof(uint32_t); ++i) {
            if (states[i] == TREELAND_FOREIGN_TOPLEVEL_HANDLE_V2_STATE_MINIMIZED) {
                m_minimized = true;
                break;
            }
        }
    }

    void treeland_foreign_toplevel_handle_v2_closed() override
    {
        // The compositor unmapped this window; the handle is inert from here
        // on and must not be used, so let the manager drop it.
        m_onClosed(this);
    }

private:
    ClosedHandler m_onClosed;
    uint32_t m_pid { 0 };
    QString m_title;
    bool m_minimized { false };
};

/*!
 * \brief Binds treeland_foreign_toplevel_manager_v2 and tracks its handles
 */
class ForeignToplevelManager : public QWaylandClientExtensionTemplate<ForeignToplevelManager>,
                               public QtWayland::treeland_foreign_toplevel_manager_v2
{
    Q_OBJECT

public:
    ForeignToplevelManager()
        : QWaylandClientExtensionTemplate(1)
    {
    }

    /*!
     * \brief Restores and focuses the toplevel belonging to \a window
     *
     * Returns false when there is nothing to ask: the compositor does not
     * expose the protocol, or it reports no toplevel for \a window.
     */
    bool restoreAndActivate(QWindow *window)
    {
        auto *waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        if (!waylandApp || !isActive())
            return false;

        ForeignToplevelHandle *target = handleFor(window);
        if (!target) {
            qCDebug(logDFMBase) << "No foreign toplevel for window:" << window->title();
            return false;
        }

        qCInfo(logDFMBase) << "Restoring foreign toplevel, pid:" << target->pid()
                           << "minimized:" << target->isMinimized()
                           << "title:" << target->title();
        target->restoreAndActivate(waylandApp->seat());

        // Requests stay buffered until the display is flushed, which Qt only
        // does on its next event loop iteration; the window has to come back now.
        wl_display_flush(waylandApp->display());
        return true;
    }

protected:
    void treeland_foreign_toplevel_manager_v2_toplevel(::treeland_foreign_toplevel_handle_v2 *toplevel) override
    {
        m_handles.append(new ForeignToplevelHandle(toplevel, [this](ForeignToplevelHandle *handle) {
            m_handles.removeAll(handle);
            delete handle;
        }));
    }

    void treeland_foreign_toplevel_manager_v2_finished() override
    {
        qCInfo(logDFMBase) << "Treeland foreign toplevel manager finished";
    }

private:
    /*!
     * \brief Picks the handle that belongs to \a window
     *
     * The protocol never maps a handle back to a wl_surface, so a window is
     * identified by this process' pid plus the title the compositor mirrors
     * from xdg-toplevel. That title lags behind the window itself and opening a
     * tab changes it right before activation, so a title match is preferred but
     * an unmatched one falls back to the only minimized toplevel - the state a
     * restore has to lift.
     */
    ForeignToplevelHandle *handleFor(const QWindow *window) const
    {
        const uint32_t selfPid = QCoreApplication::applicationPid();
        QList<ForeignToplevelHandle *> own;
        for (auto *handle : m_handles) {
            if (handle->pid() == selfPid)
                own.append(handle);
        }
        if (own.isEmpty())
            return nullptr;

        const QStringList &titles = compositorTitles(window);
        for (auto *handle : std::as_const(own)) {
            if (titles.contains(handle->title()) && handle->isMinimized())
                return handle;
        }
        for (auto *handle : std::as_const(own)) {
            if (titles.contains(handle->title()))
                return handle;
        }

        if (own.size() == 1)
            return own.first();

        ForeignToplevelHandle *minimized = nullptr;
        for (auto *handle : std::as_const(own)) {
            if (!handle->isMinimized())
                continue;
            if (minimized)
                return nullptr;   // ambiguous, leave it to xdg-activation
            minimized = handle;
        }
        return minimized;
    }

    /*!
     * \brief The titles with which the compositor may report \a window
     *
     * QWaylandWindow::setWindowTitle() appends " — <display name>" unless the
     * title already ends with it, and a handle only carries that formatted
     * form, so both spellings have to be accepted.
     */
    static QStringList compositorTitles(const QWindow *window)
    {
        QStringList titles { window->title() };
        const QString &displayName = QGuiApplication::applicationDisplayName();
        if (!displayName.isEmpty() && !window->title().endsWith(displayName)) {
            titles << (window->title().isEmpty()
                               ? displayName
                               : window->title() + QStringLiteral(" \u2014 ") + displayName);
        }
        return titles;
    }

    QList<ForeignToplevelHandle *> m_handles;
};

/*!
 * \brief Process wide manager, created on first use and never destroyed
 *
 * It is bound to the Qt Wayland display and lives as long as that connection;
 * tearing it down during shutdown would destroy proxies after the connection
 * is already gone. The native interface only exists when the Wayland platform
 * plugin is in use, and creating the extension without it would open a second
 * connection to the compositor.
 */
ForeignToplevelManager *manager()
{
    static ForeignToplevelManager *instance = nullptr;
    if (!instance && qGuiApp && qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        instance = new ForeignToplevelManager;
    return instance;
}

}   // namespace

void bindToplevelManager()
{
    manager();
}

bool restoreAndActivate(QWindow *window)
{
    if (!window)
        return false;

    auto *instance = manager();
    return instance && instance->restoreAndActivate(window);
}

}   // namespace windowactivator
DFMBASE_END_NAMESPACE

#    include "windowactivator_p.moc"

#else

DFMBASE_BEGIN_NAMESPACE
namespace windowactivator {

void bindToplevelManager()
{
}

bool restoreAndActivate(QWindow *)
{
    return false;
}

}   // namespace windowactivator
DFMBASE_END_NAMESPACE

#endif   // DFM_WITH_TREELAND_TOPLEVEL
