// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowactivator_p.h"

#if defined(DFM_WITH_FOREIGN_TOPLEVEL)

#    include <QCoreApplication>
#    include <QDebug>
#    include <QFileInfo>
#    include <QGuiApplication>
#    include <QPointer>
#    include <QStringList>
#    include <QTimer>
#    include <QWindow>

#    include <QtGui/qguiapplication_platform.h>
#    include <QtWaylandClient/QWaylandClientExtension>

#    include "qwayland-wlr-foreign-toplevel-management-unstable-v1.h"

#    include <algorithm>
#    include <functional>
#    include <utility>
#    include <wayland-client-core.h>
#    include <wayland-util.h>

DFMBASE_BEGIN_NAMESPACE
namespace windowactivator {
namespace {

/*!
 * \brief One toplevel as described by the compositor
 *
 * zwlr_foreign_toplevel_manager_v1 reports every mapped toplevel of every
 * client, so the fields needed to recognise one of our own windows - the
 * app_id and the mirrored title, plus the minimized state - are cached here.
 * The protocol has no pid event, so a window is identified by app_id and title.
 */
class ForeignToplevelHandle : public QtWayland::zwlr_foreign_toplevel_handle_v1
{
public:
    using ClosedHandler = std::function<void(ForeignToplevelHandle *)>;

    ForeignToplevelHandle(::zwlr_foreign_toplevel_handle_v1 *object, ClosedHandler onClosed)
        : QtWayland::zwlr_foreign_toplevel_handle_v1(object), m_onClosed(std::move(onClosed))
    {
    }

    ~ForeignToplevelHandle() override
    {
        if (isInitialized())
            destroy();
    }

    const QString &appId() const { return m_appId; }
    const QString &title() const { return m_title; }
    bool isMinimized() const { return m_minimized; }

    void restoreAndActivate(::wl_seat *seat)
    {
        if (m_minimized)
            unset_minimized();
        if (seat)
            activate(seat);
    }

protected:
    void zwlr_foreign_toplevel_handle_v1_app_id(const QString &appId) override
    {
        m_appId = appId;
    }

    void zwlr_foreign_toplevel_handle_v1_title(const QString &title) override
    {
        m_title = title;
    }

    void zwlr_foreign_toplevel_handle_v1_state(struct ::wl_array *state) override
    {
        m_minimized = false;
        if (!state || !state->data)
            return;

        const auto *states = static_cast<const uint32_t *>(state->data);
        for (size_t i = 0; i < state->size / sizeof(uint32_t); ++i) {
            if (states[i] == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED) {
                m_minimized = true;
                break;
            }
        }
    }

    void zwlr_foreign_toplevel_handle_v1_closed() override
    {
        // The compositor unmapped this window; the handle is inert from here
        // on and must not be used, so let the manager drop it.
        m_onClosed(this);
    }

private:
    ClosedHandler m_onClosed;
    QString m_appId;
    QString m_title;
    bool m_minimized { false };
};

/*!
 * \brief Binds zwlr_foreign_toplevel_manager_v1 and tracks its handles
 */
class ForeignToplevelManager : public QWaylandClientExtensionTemplate<ForeignToplevelManager>,
                               public QtWayland::zwlr_foreign_toplevel_manager_v1
{
    Q_OBJECT

public:
    ForeignToplevelManager()
        : QWaylandClientExtensionTemplate(3)
    {
    }

    /*!
     * \brief Restores and focuses the toplevel belonging to \a window
     *
     * Returns false only when the compositor does not expose the protocol, so
     * the caller can fall back. Otherwise the window is handled here, though
     * possibly not before the next event loop turns: a window that was just
     * created has no toplevel yet, and one whose title was just changed is
     * still reported under the old title, so the handle cannot be picked right
     * away. Asking again once that has settled keeps multiple windows apart
     * without having to guess.
     */
    bool restoreAndActivate(QWindow *window)
    {
        if (!isActive() || !qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
            return false;

        if (restoreNow(window))
            return true;

        scheduleRetry(QPointer<QWindow>(window), kRetryAttempts);
        return true;
    }

protected:
    void zwlr_foreign_toplevel_manager_v1_toplevel(::zwlr_foreign_toplevel_handle_v1 *toplevel) override
    {
        m_handles.append(new ForeignToplevelHandle(toplevel, [this](ForeignToplevelHandle *handle) {
            m_handles.removeAll(handle);
            delete handle;
        }));
    }

    void zwlr_foreign_toplevel_manager_v1_finished() override
    {
        qCInfo(logDFMBase) << "Foreign toplevel manager finished";
    }

private:
    static constexpr int kRetryAttempts = 6;
    static constexpr int kRetryDelayMs = 40;

    /*!
     * \brief Restores \a window if its toplevel is known right now
     */
    bool restoreNow(QWindow *window)
    {
        ForeignToplevelHandle *target = handleFor(window);
        if (!target)
            return false;

        auto *waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        if (!waylandApp)
            return false;

        qCInfo(logDFMBase) << "Restoring foreign toplevel, app_id:" << target->appId()
                           << "minimized:" << target->isMinimized()
                           << "title:" << target->title();
        target->restoreAndActivate(waylandApp->seat());

        // Requests stay buffered until the display is flushed, which Qt only
        // does on its next event loop iteration; the window has to come back now.
        wl_display_flush(waylandApp->display());
        return true;
    }

    /*!
     * \brief Tries restoreNow() again until the compositor catches up
     */
    void scheduleRetry(QPointer<QWindow> window, int attemptsLeft)
    {
        if (attemptsLeft <= 0) {
            qCWarning(logDFMBase) << "No foreign toplevel appeared for window:"
                                  << (window ? window->title() : QString());
            return;
        }

        QTimer::singleShot(kRetryDelayMs, this, [this, window, attemptsLeft] {
            if (!window)
                return;
            if (!restoreNow(window))
                scheduleRetry(window, attemptsLeft - 1);
        });
    }

    /*!
     * \brief Picks the handle that belongs to \a window
     *
     * Only handles carrying our app_id are considered, since the protocol does
     * not say which client a toplevel belongs to. Among those, the title is what
     * identifies a window - but the compositor mirrors it with a delay, and
     * opening a tab renames the window right before it is activated. So when no
     * title matches, the handles known to belong to our other windows are taken
     * out of the picture first: whatever is left is this one, even while the
     * compositor still reports it under its previous name.
     */
    ForeignToplevelHandle *handleFor(const QWindow *window) const
    {
        const QStringList &appIds = compositorAppIds();
        QList<ForeignToplevelHandle *> own;
        for (auto *handle : m_handles) {
            if (appIds.contains(handle->appId()))
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

        // No title match: drop the handles that belong to our other windows,
        // then accept what remains if it can only be this window.
        const QStringList &others = otherWindowsTitles(window);
        QList<ForeignToplevelHandle *> unclaimed;
        for (auto *handle : std::as_const(own)) {
            if (!others.contains(handle->title()))
                unclaimed.append(handle);
        }
        return soleMatch(unclaimed, true);
    }

    /*!
     * \brief The only handle among \a candidates that can be the wanted window
     *
     * Returns null when there is more than one, because picking then would mean
     * restoring an arbitrary window - better to report failure and let the
     * caller fall back than to raise the wrong window.
     */
    static ForeignToplevelHandle *soleMatch(const QList<ForeignToplevelHandle *> &candidates,
                                            bool preferMinimized)
    {
        if (candidates.size() == 1)
            return candidates.first();

        if (!preferMinimized)
            return nullptr;

        ForeignToplevelHandle *onlyMinimized = nullptr;
        for (auto *handle : candidates) {
            if (!handle->isMinimized())
                continue;
            if (onlyMinimized)
                return nullptr;
            onlyMinimized = handle;
        }
        return onlyMinimized;
    }

    /*!
     * \brief The titles our other live windows currently report
     *
     * Used to tell the toplevels of this application's other windows apart when
     * \a window's own title cannot be matched yet.
     */
    static QStringList otherWindowsTitles(const QWindow *window)
    {
        QStringList titles;
        for (QWindow *other : QGuiApplication::allWindows()) {
            if (other == window)
                continue;
            const QStringList &candidates = compositorTitles(other);
            for (const QString &title : candidates) {
                if (!titles.contains(title))
                    titles.append(title);
            }
        }
        return titles;
    }

    /*!
     * \brief The app_ids under which the compositor may report our windows
     *
     * QWaylandWindow derives the app_id from the desktop file name, falling
     * back to the executable name optionally prefixed with the reversed
     * organization domain, so all of those spellings have to be accepted.
     */
    static QStringList compositorAppIds()
    {
        const QString &desktopFileName = QGuiApplication::desktopFileName();
        if (!desktopFileName.isEmpty())
            return { desktopFileName };

        QStringList names { QCoreApplication::applicationName() };
        const QString &executable = QCoreApplication::applicationFilePath();
        if (!executable.isEmpty()) {
            const QString &baseName = QFileInfo(executable).baseName();
            if (!names.contains(baseName))
                names << baseName;
        }

        // Qt prepends the organization domain component by component, so the
        // resulting app_id carries the domain in reverse order.
        const QStringList &domain = QCoreApplication::organizationDomain().split(
                QLatin1Char('.'), Qt::SkipEmptyParts);
        if (domain.isEmpty())
            return names;

        QStringList reversed = domain;
        std::reverse(reversed.begin(), reversed.end());
        const QString &prefix = reversed.join(QLatin1Char('.')) + QLatin1Char('.');

        QStringList appIds = names;
        for (const QString &name : std::as_const(names))
            appIds << prefix + name;
        return appIds;
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
    if (!instance && qGuiApp
        && qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()) {
        instance = new ForeignToplevelManager;
    }
    return instance;
}

}   // namespace

void prepare()
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

void prepare()
{
}

bool restoreAndActivate(QWindow *)
{
    return false;
}

}   // namespace windowactivator
DFMBASE_END_NAMESPACE

#endif   // DFM_WITH_FOREIGN_TOPLEVEL
