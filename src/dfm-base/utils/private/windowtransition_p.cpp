// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowtransition_p.h"

#include <QImage>
#include <QPainter>

DFMBASE_BEGIN_NAMESPACE
namespace windowtransition {

/*!
 * \brief The visible (non-transparent) part of an icon as painted into a cell
 *
 * Replicates IconPainterUtils::paintIcon geometry: clamp the icon pixmap to
 * the cell and center it, then trim the icon's own transparent padding.
 */
QRectF paintedIconRect(const QIcon &icon, const QRectF &cell, QPixmap *croppedPixmap)
{
    if (icon.isNull() || cell.isEmpty())
        return QRectF();

    QPixmap pixmap = icon.pixmap(cell.size().toSize());
    if (pixmap.isNull())
        return QRectF();

    const qreal dpr = pixmap.devicePixelRatio();
    QSizeF size(pixmap.width() / dpr, pixmap.height() / dpr);

    // Same geometry as IconPainterUtils::paintIcon: clamp to the cell ...
    if (size.width() > cell.width() || size.height() > cell.height()) {
        const qreal scale = qMin(cell.width() / size.width(), cell.height() / size.height());
        size *= scale;
    }
    // ... and center it (Qt::AlignCenter).
    const QPointF drawPos = cell.topLeft() + QPointF((cell.width() - size.width()) / 2.0,
                                                     (cell.height() - size.height()) / 2.0);

    const QRect bounds = alphaBoundingRect(pixmap);
    if (bounds.isEmpty())
        return QRectF();

    if (croppedPixmap)
        *croppedPixmap = pixmap.copy(bounds);

    QRectF visible(drawPos + QPointF(bounds.x() / dpr, bounds.y() / dpr),
                   QSizeF(bounds.width() / dpr, bounds.height() / dpr));
    return visible.intersected(cell);
}

QRect alphaBoundingRect(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage();
    if (image.isNull())
        return QRect();
    if (!image.hasAlphaChannel())
        return image.rect();

    image = image.convertToFormat(QImage::Format_ARGB32);
    int minX = image.width(), minY = image.height(), maxX = -1, maxY = -1;
    for (int y = 0; y < image.height(); ++y) {
        const uchar *row = image.constScanLine(y);
        for (int x = 0; x < image.width(); ++x) {
            if (row[x * 4 + 3] != 0) {
                minX = qMin(minX, x);
                maxX = qMax(maxX, x);
                minY = qMin(minY, y);
                maxY = qMax(maxY, y);
            }
        }
    }
    if (maxX < 0)
        return QRect();
    return QRect(QPoint(minX, minY), QPoint(maxX, maxY));
}

QPixmap cropPixmapTo(const QPixmap &pixmap, const QRectF &fullRect, const QRect &subRect)
{
    if (pixmap.isNull() || fullRect.isEmpty() || subRect.isEmpty())
        return QPixmap();

    // The pixmap depicts fullRect (logical units) at its device-pixel size;
    // scale the sub-rectangle accordingly. dpr divides out in the ratios.
    const qreal sx = fullRect.width() > 0 ? pixmap.width() / fullRect.width() : 0;
    const qreal sy = fullRect.height() > 0 ? pixmap.height() / fullRect.height() : 0;
    if (sx <= 0 || sy <= 0)
        return QPixmap();

    const QRect crop(QPoint(qRound((subRect.left() - fullRect.left()) * sx),
                            qRound((subRect.top() - fullRect.top()) * sy)),
                     QSize(qRound(subRect.width() * sx), qRound(subRect.height() * sy)));
    return pixmap.copy(crop.intersected(pixmap.rect()));
}

}   // namespace windowtransition
DFMBASE_END_NAMESPACE

#if defined(DFM_WITH_TREELAND_TRANSITION)

#    include <QDebug>
#    include <QGuiApplication>
#    include <QObject>
#    include <QWindow>

#    include <QtGui/qguiapplication_platform.h>
#    include <QtWaylandClient/QWaylandClientExtension>
#    include <private/qwaylanddisplay_p.h>
#    include <private/qwaylandshmbackingstore_p.h>
#    include <private/qwaylandwindow_p.h>

#    include "qwayland-treeland-window-transition-unstable-v1.h"
#    include "qwayland-xdg-activation-v1.h"

#    include <wayland-client.h>

#    include <QLocalServer>
#    include <QLocalSocket>
#    include <QTimer>
#    include <memory>

DFMBASE_BEGIN_NAMESPACE
namespace windowtransition {
namespace {

/*!
 * \brief Binds xdg_activation_v1 (token creation, window activation)
 */
class XdgActivationManager : public QWaylandClientExtensionTemplate<XdgActivationManager>,
                             public QtWayland::xdg_activation_v1
{
    Q_OBJECT
public:
    explicit XdgActivationManager()
        : QWaylandClientExtensionTemplate<XdgActivationManager>(1)
    {
    }

    ~XdgActivationManager() override
    {
        if (isInitialized())
            destroy();
    }
};

/*!
 * \brief Binds treeland_window_transition_manager_v1 (transition rectangles)
 */
class WindowTransitionManager : public QWaylandClientExtensionTemplate<WindowTransitionManager>,
                                public QtWayland::treeland_window_transition_manager_v1
{
    Q_OBJECT
public:
    explicit WindowTransitionManager()
        : QWaylandClientExtensionTemplate<WindowTransitionManager>(1)
    {
    }

    ~WindowTransitionManager() override
    {
        if (isInitialized())
            destroy();
    }
};

/*!
 * \brief An activation token; its string arrives through the done event
 */
class ActivationToken : public QObject, public QtWayland::xdg_activation_token_v1
{
    Q_OBJECT
public:
    ActivationToken(::xdg_activation_token_v1 *object, QObject *parent = nullptr)
        : QObject(parent), QtWayland::xdg_activation_token_v1(object)
    {
    }

protected:
    void xdg_activation_token_v1_done(const QString &token) override
    {
        m_token = token;
        Q_EMIT done(token);
    }

Q_SIGNALS:
    void done(const QString &token);

private:
    QString m_token;
};

/*!
 * \brief A transition rectangle, optionally with a live source surface
 */
class WindowTransitionRect : public QObject, public QtWayland::treeland_window_transition_rect_v1
{
    Q_OBJECT
public:
    WindowTransitionRect(::treeland_window_transition_rect_v1 *object, QObject *parent = nullptr)
        : QObject(parent), QtWayland::treeland_window_transition_rect_v1(object)
    {
    }

    ~WindowTransitionRect() override
    {
        if (m_sourceSurface)
            wl_surface_destroy(m_sourceSurface);
        delete m_shmBuffer;
    }

    /*!
     * \brief Draws \a pixmap into a role-less surface and attaches it as the
     * live source of the transition
     */
    void setSourceImage(QtWaylandClient::QWaylandDisplay *display, const QPixmap &pixmap)
    {
        if (!display || !display->compositor())
            return;

        if (!m_sourceSurface)
            m_sourceSurface = display->compositor()->create_surface();

        delete m_shmBuffer;
        m_shmBuffer = new QtWaylandClient::QWaylandShmBuffer(display,
                                                             pixmap.size(),
                                                             QImage::Format_ARGB32_Premultiplied);
        QImage *image = m_shmBuffer->image();
        image->fill(Qt::transparent);
        QPainter painter(image);
        painter.drawPixmap(0, 0, pixmap);
        painter.end();

        // The compositor renders this surface live during the transition, so
        // it must be kept alive with valid content until the rect is closed.
        wl_surface_attach(m_sourceSurface, m_shmBuffer->buffer(), 0, 0);
        wl_surface_damage(m_sourceSurface, 0, 0, pixmap.width(), pixmap.height());
        wl_surface_commit(m_sourceSurface);

        set_source_surface(m_sourceSurface);
    }

    void clearSourceImage()
    {
        set_source_surface(nullptr);
    }

protected:
    void treeland_window_transition_rect_v1_closed() override
    {
        Q_EMIT closed();
    }

Q_SIGNALS:
    void closed();

private:
    ::wl_surface *m_sourceSurface { nullptr };
    QtWaylandClient::QWaylandShmBuffer *m_shmBuffer { nullptr };
};

/*!
 * \brief Process-wide managers, created on first use and never destroyed
 *
 * They are bound to the Qt Wayland display and live as long as that connection;
 * the native interface only exists when the Wayland platform plugin is in use,
 * and creating an extension without it would open a second connection to the
 * compositor. The registry has already been read by the time the application
 * runs, so the globals - when exposed - initialize the managers immediately.
 */
XdgActivationManager *activationManager()
{
    static XdgActivationManager *instance = nullptr;
    if (!instance && qGuiApp && qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        instance = new XdgActivationManager;
    return instance;
}

WindowTransitionManager *transitionManager()
{
    static WindowTransitionManager *instance = nullptr;
    if (!instance && qGuiApp && qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>())
        instance = new WindowTransitionManager;
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

/*!
 * \brief Sends buffered requests to the compositor now
 *
 * The rectangle and its geometry must reach the compositor before the target
 * window activates, and the activation before its first map commit - with the
 * single-instance preview the target may activate milliseconds after the token
 * is handed over, long before this process' event loop flushes again. Use the
 * display's own flush, which also drains Qt's internal request queue (a plain
 * wl_display_flush only pushes the wire buffer).
 */
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

}   // namespace

class WindowTransitionContextPrivate : public QObject
{
    Q_OBJECT
public:
    explicit WindowTransitionContextPrivate(WindowTransitionContext *qq)
        : QObject(qq), q(qq)
    {
    }

    ~WindowTransitionContextPrivate() override
    {
        // Destroying the rectangle makes the compositor fall back to its
        // default close animation for a target window that is still alive.
        if (m_rect) {
            m_rect->destroy();
            m_rect = nullptr;
        }
    }

    bool start(QWindow *sourceWindow, const QRect &anchor, const QPixmap &sourcePixmap)
    {
        auto *actMgr = activationManager();
        auto *transitionMgr = transitionManager();
        if (!actMgr || !actMgr->isInitialized() || !transitionMgr || !transitionMgr->isInitialized()) {
            qCWarning(logDFMBase) << "Window transition managers not ready:"
                                  << "activation" << (actMgr ? actMgr->isInitialized() : false)
                                  << "transition" << (transitionMgr ? transitionMgr->isInitialized() : false);
            return false;
        }

        ::wl_surface *surface = windowSurface(sourceWindow);
        if (!surface)
            return false;

        // Create the activation token, identify the originating surface and
        // commit it. The token string is delivered by the done event, after
        // which the transition rectangle can be created for it.
        auto *rawToken = actMgr->get_activation_token();
        if (!rawToken)
            return false;
        m_token = new ActivationToken(rawToken, this);
        connect(m_token, &ActivationToken::done, this, &WindowTransitionContextPrivate::onTokenDone);
        m_token->set_surface(surface);
        m_token->commit();

        m_anchor = anchor;
        m_sourcePixmap = sourcePixmap;
        m_display = windowDisplay(sourceWindow);
        m_manager = transitionMgr;
        return true;
    }

    bool isValid() const
    {
        return m_rect != nullptr;
    }

    void onTokenDone(const QString &tokenString)
    {
        if (!m_manager || !m_manager->isInitialized()) {
            qCWarning(logDFMBase) << "Window transition manager gone before the token arrived";
            return;   // nothing to attach the rectangle to; the caller's fallback takes over
        }

        auto *rawRect = m_manager->get_window_transition_rect(tokenString);
        if (!rawRect) {
            qCWarning(logDFMBase) << "Failed to create window transition rectangle";
            return;
        }

        // The rectangle stays alive (associated with the target window) so it
        // can drive that window's close animation.
        m_rect = new WindowTransitionRect(rawRect, this);
        connect(m_rect, &WindowTransitionRect::closed, this, &WindowTransitionContextPrivate::onClosed);
        m_rect->set_geometry(m_anchor.x(), m_anchor.y(), m_anchor.width(), m_anchor.height());

        if (m_display) {
            if (m_sourcePixmap.isNull())
                m_rect->clearSourceImage();
            else
                m_rect->setSourceImage(m_display, m_sourcePixmap);
        }

        // Make sure the rectangle and its geometry reach the compositor before
        // the token is handed to the target window, which may activate as soon
        // as it starts.
        flushRequests(m_display);

        qCInfo(logDFMBase) << "Window transition rectangle attached:" << m_anchor;
        Q_EMIT q->activationTokenReady(tokenString);
    }

    void updateAnchor(const QRect &anchor)
    {
        if (!m_rect)
            return;
        m_anchor = anchor;
        m_rect->set_geometry(anchor.x(), anchor.y(), anchor.width(), anchor.height());
        flushRequests(m_display);
    }

    void updateSourcePixmap(const QPixmap &pixmap)
    {
        if (!m_rect || !m_display)
            return;
        m_sourcePixmap = pixmap;
        if (pixmap.isNull())
            m_rect->clearSourceImage();
        else
            m_rect->setSourceImage(m_display, pixmap);
        flushRequests(m_display);
    }

    void onClosed()
    {
        // The compositor is done with the rectangle; it is unusable from here
        // on and must be destroyed.
        if (m_rect) {
            m_rect->destroy();
            m_rect = nullptr;
        }
        Q_EMIT q->transitionClosed();
    }

private:
    WindowTransitionContext *q { nullptr };
    WindowTransitionManager *m_manager { nullptr };
    QtWaylandClient::QWaylandDisplay *m_display { nullptr };
    ActivationToken *m_token { nullptr };
    WindowTransitionRect *m_rect { nullptr };
    QRect m_anchor;
    QPixmap m_sourcePixmap;
};

WindowTransitionContext *WindowTransitionContext::create(QWindow *sourceWindow,
                                                         const QRect &anchor,
                                                         const QPixmap &sourcePixmap,
                                                         QObject *parent)
{
    if (!sourceWindow || anchor.isEmpty())
        return nullptr;

    auto *context = new WindowTransitionContext(parent);
    if (!context->d->start(sourceWindow, anchor, sourcePixmap)) {
        delete context;
        return nullptr;
    }
    return context;
}

WindowTransitionContext::WindowTransitionContext(QObject *parent)
    : QObject(parent), d(new WindowTransitionContextPrivate(this))
{
}

WindowTransitionContext::~WindowTransitionContext() = default;

void WindowTransitionContext::updateAnchor(const QRect &anchorInWindow)
{
    d->updateAnchor(anchorInWindow);
}

void WindowTransitionContext::updateSourcePixmap(const QPixmap &sourcePixmap)
{
    d->updateSourcePixmap(sourcePixmap);
}

bool WindowTransitionContext::isValid() const
{
    return d->isValid();
}

void bindManagers()
{
    activationManager();
    transitionManager();
}

bool activateWithToken(QWindow *window, const QString &token)
{
    if (!window || token.isEmpty())
        return false;

    auto *actMgr = activationManager();
    if (!actMgr)
        return false;

    ::wl_surface *surface = windowSurface(window);
    if (!surface)
        return false;

    // The activation must reach the compositor before the first commit that
    // maps the window. The manager is created lazily and may still need one
    // event loop turn to bind its global, so when it is not ready yet the
    // request is deferred to its activation - still ahead of the first commit,
    // which waits for the compositor's configure round trip.
    if (!actMgr->isInitialized()) {
        QObject::connect(actMgr, &XdgActivationManager::activeChanged, window, [actMgr, window, token] {
            if (!actMgr->isInitialized())
                return;
            ::wl_surface *surface = windowSurface(window);
            if (!surface)
                return;
            actMgr->activate(token, surface);
            flushRequests(windowDisplay(window));
            qCInfo(logDFMBase) << "Activated window with transition token (deferred)";
        });
        return true;
    }

    actMgr->activate(token, surface);
    flushRequests(windowDisplay(window));
    qCInfo(logDFMBase) << "Activated window with transition token (now)";
    return true;
}

class WindowTransitionLaunchPrivate : public QObject
{
    Q_OBJECT
public:
    explicit WindowTransitionLaunchPrivate(WindowTransitionLaunch *qq)
        : QObject(qq), q(qq)
    {
    }

    bool start(QWindow *sourceWindow,
               const QUrl &initialUrl,
               const WindowTransitionLaunch::AnchorProvider &provider,
               const WindowTransitionLaunch::TokenCallback &onToken)
    {
        m_sourceWindow = sourceWindow;
        m_url = initialUrl;
        m_provider = provider;
        m_onToken = onToken;

        QRect anchor;
        QPixmap snapshot;
        if (!m_provider(m_url, &anchor, &snapshot) || anchor.isEmpty()) {
            qCWarning(logDFMBase) << "Window transition launch: anchor provider failed";
            return false;
        }

        m_context = WindowTransitionContext::create(m_sourceWindow, anchor, snapshot, this);
        if (!m_context) {
            qCWarning(logDFMBase) << "Window transition launch: failed to create transition context"
                                  << "anchor:" << anchor << "sourceWindow:" << (m_sourceWindow ? m_sourceWindow->title() : QString());
            return false;
        }

        // The transition channel: the preview process reports the currently
        // shown file over this local socket, so the close animation follows
        // the file the user switched to.
        startChannel();

        // Deliver the token exactly once: as soon as the compositor accepts
        // the rectangle, or empty when it never arrives (3s timeout), so the
        // caller can still launch the target window without an animation.
        auto delivered = std::make_shared<bool>(false);
        auto deliver = [this, delivered](const QString &token) {
            if (*delivered)
                return;
            *delivered = true;
            m_onToken(token, m_channelName);
        };

        connect(m_context, &WindowTransitionContext::activationTokenReady, this,
                [this, deliver](const QString &token) {
                    if (m_fallback) {
                        m_fallback->stop();
                        m_fallback->deleteLater();
                        m_fallback = nullptr;
                    }
                    deliver(token);
                });
        m_fallback = new QTimer(this);
        m_fallback->setSingleShot(true);
        connect(m_fallback, &QTimer::timeout, this, [this, deliver] {
            m_fallback->deleteLater();
            m_fallback = nullptr;
            // The target window launched without the token: nobody will ever
            // activate the rectangle, so drop it and this launch instead of
            // polling until the compositor's pending-rect sweep.
            if (m_context) {
                m_context->deleteLater();
                m_context = nullptr;
            }
            qCWarning(logDFMBase) << "Window transition launch: activation token never arrived, launching without animation";
            deliver(QString());
            q->deleteLater();
        });
        m_fallback->start(3000);

        // The compositor is done with the rectangle once the target window
        // closed; the launch (and its update timer) is obsolete then.
        connect(m_context, &WindowTransitionContext::transitionClosed, q, &WindowTransitionLaunch::deleteLater);

        m_lastSnapshotSize = snapshot.size();
        m_timer = new QTimer(this);
        m_timer->setInterval(100);
        connect(m_timer, &QTimer::timeout, this, &WindowTransitionLaunchPrivate::update);
        m_timer->start();
        return true;
    }

    void setCurrentFile(const QUrl &url)
    {
        if (m_url == url)
            return;
        m_url = url;
        m_urlDirty = true;
        update();   // apply right away instead of waiting for the poll
    }

    void update()
    {
        if (!m_context)
            return;

        QRect anchor;
        QPixmap snapshot;
        if (!m_provider(m_url, &anchor, &snapshot) || anchor.isEmpty()) {
            // The anchor item left the visible area (scrolled away) or is gone
            // (deleted): disable the rectangle, so the compositor falls back to
            // its default close animation instead of shrinking the window
            // toward a stale position.
            if (!m_disabled) {
                m_context->updateAnchor(QRect());
                m_disabled = true;
            }
            return;
        }

        m_disabled = false;
        m_context->updateAnchor(anchor);
        // A file switch must refresh the live source even when the new icon
        // happens to have the same size as the old one; icon size changes do
        // the same through the anchor size change.
        if (!snapshot.isNull() && (m_urlDirty || anchor.size() != m_lastSnapshotSize)) {
            m_context->updateSourcePixmap(snapshot);
            m_lastSnapshotSize = anchor.size();
        }
        m_urlDirty = false;
    }

    void startChannel()
    {
        static int channelCounter = 0;
        m_channelName = QStringLiteral("dfm-preview-transition-%1-%2")
                            .arg(QCoreApplication::applicationPid())
                            .arg(++channelCounter);
        m_server = new QLocalServer(this);
        QLocalServer::removeServer(m_channelName);
        if (!m_server->listen(m_channelName)) {
            qCWarning(logDFMBase) << "Failed to listen on transition channel" << m_channelName;
            m_channelName.clear();
            return;
        }
        connect(m_server, &QLocalServer::newConnection, this, &WindowTransitionLaunchPrivate::onChannelConnection);
    }

    void onChannelConnection()
    {
        QLocalSocket *socket = m_server->nextPendingConnection();
        if (!socket)
            return;
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        connect(socket, &QLocalSocket::readyRead, this, [this, socket] {
            // The preview process reports one file per message; the last
            // complete one wins.
            m_channelBuffer.append(socket->readAll());
            int lastNewline = m_channelBuffer.lastIndexOf('\n');
            if (lastNewline < 0)
                return;
            const QByteArray message = m_channelBuffer.left(lastNewline).trimmed();
            m_channelBuffer.remove(0, lastNewline + 1);
            const QUrl url = QUrl::fromEncoded(message);
            if (url.isValid())
                q->setCurrentFile(url);
        });
    }

private:
    WindowTransitionLaunch *q { nullptr };
    QWindow *m_sourceWindow { nullptr };
    QUrl m_url;
    WindowTransitionLaunch::AnchorProvider m_provider;
    WindowTransitionLaunch::TokenCallback m_onToken;
    WindowTransitionContext *m_context { nullptr };
    QTimer *m_timer { nullptr };
    QTimer *m_fallback { nullptr };
    QLocalServer *m_server { nullptr };
    QString m_channelName;
    QByteArray m_channelBuffer;
    QSize m_lastSnapshotSize;
    bool m_disabled { false };
    bool m_urlDirty { false };
};

WindowTransitionLaunch *WindowTransitionLaunch::create(QWindow *sourceWindow,
                                                       const QUrl &initialUrl,
                                                       const AnchorProvider &provider,
                                                       const TokenCallback &onToken,
                                                       QObject *parent)
{
    if (!sourceWindow || !provider || !onToken)
        return nullptr;

    auto *launch = new WindowTransitionLaunch(parent);
    if (!launch->d->start(sourceWindow, initialUrl, provider, onToken)) {
        delete launch;
        return nullptr;
    }
    return launch;
}

void WindowTransitionLaunch::setCurrentFile(const QUrl &url)
{
    d->setCurrentFile(url);
}

WindowTransitionLaunch::WindowTransitionLaunch(QObject *parent)
    : QObject(parent), d(new WindowTransitionLaunchPrivate(this))
{
}

WindowTransitionLaunch::~WindowTransitionLaunch() = default;


}   // namespace windowtransition
DFMBASE_END_NAMESPACE

#    include "windowtransition_p.moc"

#else

DFMBASE_BEGIN_NAMESPACE
namespace windowtransition {

// Minimal definitions so the pimpl members can be destroyed without the
// Wayland code (which lives in the branch above).
class WindowTransitionContextPrivate : public QObject
{
};
class WindowTransitionLaunchPrivate : public QObject
{
};

WindowTransitionContext *WindowTransitionContext::create(QWindow *, const QRect &, const QPixmap &, QObject *)
{
    return nullptr;
}

WindowTransitionContext::WindowTransitionContext(QObject *parent)
    : QObject(parent)
{
}

WindowTransitionContext::~WindowTransitionContext() = default;

void WindowTransitionContext::updateAnchor(const QRect &)
{
}

void WindowTransitionContext::updateSourcePixmap(const QPixmap &)
{
}

bool WindowTransitionContext::isValid() const
{
    return false;
}

WindowTransitionLaunch *WindowTransitionLaunch::create(QWindow *, const QUrl &, const AnchorProvider &, const TokenCallback &, QObject *)
{
    return nullptr;
}

void WindowTransitionLaunch::setCurrentFile(const QUrl &)
{
}

WindowTransitionLaunch::WindowTransitionLaunch(QObject *parent)
    : QObject(parent)
{
}

WindowTransitionLaunch::~WindowTransitionLaunch() = default;

bool activateWithToken(QWindow *, const QString &)
{
    return false;
}

void bindManagers()
{
}

}   // namespace windowtransition
DFMBASE_END_NAMESPACE

#endif   // DFM_WITH_TREELAND_TRANSITION
