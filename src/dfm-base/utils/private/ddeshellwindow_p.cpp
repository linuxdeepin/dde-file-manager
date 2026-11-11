// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddeshellwindow_p.h"

#if defined(DFM_WITH_DDE_SHELL)

#    include <QDebug>
#    include <QGuiApplication>
#    include <QHash>
#    include <QWindow>

#    include <QtGui/qevent.h>
#    include <QtGui/qguiapplication_platform.h>
#    include <QtWaylandClient/QWaylandClientExtension>
#    include <private/qwaylanddisplay_p.h>
#    include <private/qwaylandwindow_p.h>

#    include "qwayland-treeland-dde-shell-v1.h"

#    include <wayland-client.h>

DFMBASE_BEGIN_NAMESPACE
namespace ddeshell {
namespace {

/*!
 * \brief Binds treeland_dde_shell_manager_v1 (shell surface creation)
 *
 * The compositor registers the global at version 1, which covers
 * get_shell_surface and everything the skip bit needs.
 */
class DDEShellManager : public QWaylandClientExtensionTemplate<DDEShellManager>,
                        public QtWayland::treeland_dde_shell_manager_v1
{
    Q_OBJECT
public:
    explicit DDEShellManager()
        : QWaylandClientExtensionTemplate<DDEShellManager>(1)
    {
    }

    ~DDEShellManager() override
    {
        // Never runs: the instance is process-global and intentionally not
        // deleted, mirroring the other protocol managers in dfm-base.
        if (isInitialized())
            destroy();
    }
};

/*!
 * \brief A shell surface carrying the skip-dock-preview bit
 */
class DDEShellSurface : public QObject, public QtWayland::treeland_dde_shell_surface_v1
{
    Q_OBJECT
public:
    explicit DDEShellSurface(::treeland_dde_shell_surface_v1 *object)
        : QtWayland::treeland_dde_shell_surface_v1(object)
    {
    }

    ~DDEShellSurface() override
    {
        // The protocol requires the shell surface to be destroyed before the
        // wl_surface it belongs to. The controller deletes this object from
        // the window's destroyed() handler, where the platform window is
        // still alive; when the surface was recreated underneath a live
        // window, the compositor destroyed the shell surface together with
        // it and the object was marked detached, so it is dropped silently.
        if (!m_detached)
            destroy();
    }

    void detach() { m_detached = true; }

private:
    bool m_detached { false };
};

/*!
 * \brief Process-wide manager, created on first use and never destroyed
 *
 * Like the transition and activation managers, it binds to the Qt Wayland
 * display and only exists when the Wayland platform plugin is in use; the
 * registry has already been read by the time the application runs, so the
 * global - when exposed - initializes the manager immediately.
 */
DDEShellManager *manager()
{
    static DDEShellManager *instance = nullptr;
    if (!instance && qGuiApp && qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        instance = new DDEShellManager;
    return instance;
}

/*! \brief The wl_surface backing \a window, valid on the Wayland platform only */
::wl_surface *windowSurface(QWindow *window)
{
    if (!window || !window->handle())
        return nullptr;
    auto *waylandWindow = static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    return waylandWindow ? waylandWindow->surface() : nullptr;
}

/*! \brief The display behind \a window, valid on the Wayland platform only */
QtWaylandClient::QWaylandDisplay *windowDisplay(QWindow *window)
{
    if (!window || !window->handle())
        return nullptr;
    auto *waylandWindow = static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
    return waylandWindow ? waylandWindow->display() : nullptr;
}

/*! \brief Sends buffered requests to the compositor now, see windowtransition_p */
void flushRequests(QtWaylandClient::QWaylandDisplay *display)
{
    if (display) {
        display->flushRequests();
        return;
    }
    auto *waylandApp = qGuiApp ? qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>() : nullptr;
    if (waylandApp && waylandApp->display())
        wl_display_flush(waylandApp->display());
}

/*!
 * \brief Applies the skip-dock-preview bit to windows, retrying while the
 * manager global or the platform surface are still pending
 */
class SkipDockController : public QObject
{
    Q_OBJECT
public:
    static SkipDockController *instance()
    {
        static SkipDockController controller;
        return &controller;
    }

    void apply(QWindow *window, bool skip)
    {
        Entry &entry = m_entries[window];
        entry.skip = skip;

        if (!entry.cleanupConnected) {
            entry.cleanupConnected = true;
            // Runs while the platform window is still alive, so the shell
            // surface goes in the order the protocol requires.
            connect(window, &QObject::destroyed, this, [this, window] {
                Entry entry = m_entries.take(window);
                delete entry.shellSurface;
            });
        }

        DDEShellManager *mgr = manager();
        if (!mgr)
            return;   // no Wayland platform; the window keeps its dock entry

        // The manager is created lazily and may still need one event loop
        // turn to bind its global; retry all pending windows once it arrives.
        connect(mgr, &DDEShellManager::activeChanged, this, &SkipDockController::retryAll, Qt::UniqueConnection);

        if (!mgr->isInitialized()) {
            if (!entry.loggedPending) {
                entry.loggedPending = true;
                qCDebug(logDFMBase) << "dde shell manager not bound yet, skip-dock-preview pending for" << window;
            }
            return;
        }

        ::wl_surface *surface = windowSurface(window);
        if (!surface) {
            // No platform surface yet (window never shown or being recreated);
            // watch for its creation.
            if (!entry.watchingSurface) {
                entry.watchingSurface = true;
                window->installEventFilter(this);
            }
            return;
        }

        if (entry.surface && entry.surface != surface) {
            // The wl_surface was recreated underneath the live window; the
            // compositor destroyed the shell surface together with it.
            if (entry.shellSurface) {
                entry.shellSurface->detach();
                delete entry.shellSurface;
                entry.shellSurface = nullptr;
            }
        }
        entry.surface = surface;

        if (!entry.shellSurface) {
            entry.shellSurface = new DDEShellSurface(mgr->get_shell_surface(surface));
            qCInfo(logDFMBase) << "Created dde shell surface to hide window from the dock:" << window;
        }
        entry.shellSurface->set_skip_dock_preview(skip ? 1 : 0);
        flushRequests(windowDisplay(window));
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::PlatformSurface) {
            auto *surfaceEvent = static_cast<QPlatformSurfaceEvent *>(event);
            if (surfaceEvent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                if (auto *window = qobject_cast<QWindow *>(watched)) {
                    const auto it = m_entries.constFind(window);
                    if (it != m_entries.constEnd())
                        apply(window, it->skip);
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    struct Entry
    {
        bool skip { false };
        bool cleanupConnected { false };
        bool watchingSurface { false };
        bool loggedPending { false };
        ::wl_surface *surface { nullptr };
        DDEShellSurface *shellSurface { nullptr };
    };

    void retryAll()
    {
        if (!manager() || !manager()->isInitialized())
            return;
        const auto windows = m_entries.keys();
        for (QWindow *window : windows)
            apply(window, m_entries.value(window).skip);
    }

    QHash<QWindow *, Entry> m_entries;
};

}   // namespace

bool setSkipDockPreview(QWindow *window, bool skip)
{
    if (!window)
        return false;
    if (!manager())
        return false;   // no Wayland platform: nothing to hide with

    SkipDockController::instance()->apply(window, skip);
    return true;
}

}   // namespace ddeshell
DFMBASE_END_NAMESPACE

#    include "ddeshellwindow_p.moc"

#else

DFMBASE_BEGIN_NAMESPACE
namespace ddeshell {

// Minimal definition so callers can link without the Wayland code (which
// lives in the branch above).
bool setSkipDockPreview(QWindow *, bool)
{
    return false;
}

}   // namespace ddeshell
DFMBASE_END_NAMESPACE

#endif   // DFM_WITH_DDE_SHELL
