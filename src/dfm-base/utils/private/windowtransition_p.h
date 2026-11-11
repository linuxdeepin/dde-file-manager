// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWTRANSITION_P_H
#define WINDOWTRANSITION_P_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QIcon>
#include <QPixmap>
#include <QRect>
#include <QScopedPointer>
#include <QUrl>

#include <functional>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE

namespace windowtransition {

class WindowTransitionContextPrivate;
class WindowTransitionLaunchPrivate;

/*!
 * \class WindowTransitionContext
 * \brief Drives a treeland window transition for one launched window
 *
 * Client side of treeland-window-transition-unstable-v1, mirroring treeland's
 * test-window-transition example: an activation token is created for the
 * source window, a transition rectangle (the selected item's icon) is attached
 * to it, and the token is handed to the window being launched. The compositor
 * plays the open animation from the rectangle to the target window's geometry,
 * and the close animation back to the rectangle when the target window closes.
 *
 * The rectangle coordinates are relative to the source window's surface, so
 * moving the source window needs no updates; the context keeps the rectangle
 * alive and may update it while the target window lives, which lets the close
 * animation follow the item as the view scrolls, the icon size changes or the
 * source window is resized.
 *
 * When the compositor cannot play the transition (no Wayland platform, the
 * protocol is not exposed, or an empty anchor), create() returns null and the
 * caller should launch the window without a token.
 */
class WindowTransitionContext : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Starts a window transition for a window launched with the returned token
     *
     * \param sourceWindow   the window that anchors the transition, i.e. the
     *                       file manager window whose surface owns the rectangle
     * \param anchorInWindow the anchor rectangle in \a sourceWindow widget
     *                       coordinates (the selected item's icon)
     * \param sourcePixmap   snapshot of the anchor content, rendered live by the
     *                       compositor while the transition plays; a null pixmap
     *                       disables the live source surface
     * \param parent
     *
     * Returns null when the transition cannot be set up.
     */
    static WindowTransitionContext *create(QWindow *sourceWindow,
                                           const QRect &anchorInWindow,
                                           const QPixmap &sourcePixmap,
                                           QObject *parent = nullptr);

    /*!
     * \brief The activation token to launch the target window with
     *
     * The token must reach the target window, which calls
     * xdg_activation_v1.activate before its first map. Emitted once the
     * transition rectangle has been attached to the compositor.
     */
    Q_SIGNAL void activationTokenReady(const QString &token);

    /*!
     * \brief The compositor is done with the rectangle
     *
     * Emitted after the target window closed and its close animation finished.
     * The context is unusable afterwards and should be destroyed.
     */
    Q_SIGNAL void transitionClosed();

    /*! \brief Updates the anchor rectangle, relative to the source window */
    void updateAnchor(const QRect &anchorInWindow);

    /*! \brief Replaces the live source surface content, e.g. after an icon size change */
    void updateSourcePixmap(const QPixmap &sourcePixmap);

    /*! \brief Whether the rectangle is attached to the compositor */
    bool isValid() const;

private:
    explicit WindowTransitionContext(QObject *parent = nullptr);
    ~WindowTransitionContext() override;

    QScopedPointer<WindowTransitionContextPrivate> d;

    friend class WindowTransitionContextPrivate;
};

/*!
 * \brief Binds the transition and activation managers
 *
 * The Wayland globals behind both managers are picked up lazily when the
 * managers are first created, which takes one event loop turn; call this early
 * (at window creation) so the first preview does not miss its transition.
 */
void bindManagers();

/*!
 * \brief Activates \a window with \a token
 *
 * Target side of the transition, mirroring the example's receiver: the window
 * is shown first (which creates its shell surface), then activate is called
 * before the first map commit, making the compositor play the open transition
 * attached to the token. Returns false when there is nothing to activate with,
 * in which case the caller should activate the window normally.
 *
 * When the xdg-activation global is not bound yet (fresh process before the
 * event loop turned), the activation is deferred until it arrives, which is
 * still ahead of the first commit.
 */
bool activateWithToken(QWindow *window, const QString &token);

/*!
 * \brief The transition anchor for an item rectangle
 *
 * \param itemRect               item rectangle in viewport coordinates
 * \param viewportRect           the viewport rectangle
 * \param viewportOriginInWindow the viewport origin in source window coordinates
 *
 * A partially visible item anchors at its visible part; a fully hidden item
 * returns an empty rectangle, which means the transition must be skipped.
 */
inline QRect anchorInWindow(const QRect &itemRect,
                            const QRect &viewportRect,
                            const QPoint &viewportOriginInWindow)
{
    const QRect visible = itemRect.intersected(viewportRect);
    if (visible.isEmpty())
        return QRect();
    return QRect(visible.topLeft() + viewportOriginInWindow, visible.size());
}

/*!
 * \brief The visible part of \a icon as painted into \a cell
 *
 * Replicates IconPainterUtils::paintIcon geometry: the icon pixmap is clamped
 * to the cell and centered, then trimmed to its non-transparent pixels. The
 * result is the icon the user actually sees; \a croppedPixmap receives the
 * same content cropped, ready to be uploaded as the transition's live source.
 * Returns an empty rectangle for a null icon or fully transparent pixmap.
 */
QRectF paintedIconRect(const QIcon &icon, const QRectF &cell, QPixmap *croppedPixmap = nullptr);

/*!
 * \brief The bounding box of the non-transparent pixels of \a pixmap
 *
 * Device-pixel coordinates; an opaque pixmap returns its full rectangle.
 */
QRect alphaBoundingRect(const QPixmap &pixmap);

/*!
 * \brief Crops \a pixmap, which depicts \a fullRect, down to \a subRect
 *
 * Used to keep the live source surface consistent with an anchor that was
 * clipped to the visible viewport: the result shows exactly the part of the
 * icon that the anchor rectangle covers.
 */
QPixmap cropPixmapTo(const QPixmap &pixmap, const QRectF &fullRect, const QRect &subRect);

/*!
 * \class WindowTransitionLaunch
 * \brief Runs one preview launch with a window transition
 *
 * Bundles the common client flow for a space-key preview: create the
 * transition context for \a sourceWindow from an anchor provider, deliver the
 * activation token and a transition channel name through \a onToken once the
 * compositor accepts the rectangle (or empty ones when the transition is
 * unavailable / timed out), and keep the rectangle in sync with the view
 * while the preview lives by polling the provider. The channel is a local
 * socket the preview process may connect to in order to report the currently
 * shown file, so the close animation follows the file the user switched to.
 * The object deletes itself once the compositor reports the rectangle closed.
 */
class WindowTransitionLaunch : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Computes the anchor and its cropped snapshot for \a url
     *
     * \a anchor is in \a sourceWindow coordinates. Returning false means the
     * item is not visible right now (scrolled away or deleted); the rectangle
     * is disabled then and the compositor falls back to its default close
     * animation.
     */
    using AnchorProvider = std::function<bool(const QUrl &url, QRect *anchor, QPixmap *snapshot)>;

    /*!
     * \brief Called once with the token to launch the target window with and
     * the transition channel name the target may report file switches on
     */
    using TokenCallback = std::function<void(const QString &token, const QString &channelName)>;

    /*!
     * \brief Starts a transition-backed launch anchored at \a initialUrl
     *
     * Returns null when the transition cannot be set up (no compositor
     * support, or no anchor at launch); the caller should then deliver empty
     * token and channel itself, i.e. launch the target window without
     * animation.
     */
    static WindowTransitionLaunch *create(QWindow *sourceWindow,
                                          const QUrl &initialUrl,
                                          const AnchorProvider &provider,
                                          const TokenCallback &onToken,
                                          QObject *parent = nullptr);

    /*!
     * \brief The preview switched to another file: the anchor follows it
     *
     * The provider is re-run for \a url immediately, and the live source
     * surface is refreshed even when the icon size did not change.
     */
    void setCurrentFile(const QUrl &url);

private:
    explicit WindowTransitionLaunch(QObject *parent = nullptr);
    ~WindowTransitionLaunch() override;

    QScopedPointer<WindowTransitionLaunchPrivate> d;

    friend class WindowTransitionLaunchPrivate;
};

}   // namespace windowtransition

DFMBASE_END_NAMESPACE

#endif   // WINDOWTRANSITION_P_H
