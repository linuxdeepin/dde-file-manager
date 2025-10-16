// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detailspacewidget.h"
#include "detailview.h"
#include "utils/detailspacehelper.h"

#include <dfm-framework/event/event.h>
#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QCursor>
#include <QHBoxLayout>
#include <QMouseEvent>

#include <algorithm>

using namespace dfmplugin_detailspace;

DetailSpaceWidget::DetailSpaceWidget(QFrame *parent)
    : AbstractFrame(parent)
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged,
            this, &DetailSpaceWidget::initUiForSizeMode);
#endif

    setMouseTracking(true);
    preferredWidth = defaultDetailWidth();

    initUiForSizeMode();
    initializeUi();
}

void DetailSpaceWidget::initUiForSizeMode()
{
    if (!userResized)
        preferredWidth = defaultDetailWidth();

    preferredWidth = clampWidth(preferredWidth);
    applyPreferredWidth();
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url)
{
    quint64 winId = DetailSpaceHelper::findWindowIdByDetailSpace(this);
    if (winId) {
        QList<QUrl> urls = dpfSlotChannel->push("dfmplugin_workspace", "slot_View_GetSelectedUrls", winId)
                                   .value<QList<QUrl>>();
        if (!urls.isEmpty()) {
            setCurrentUrl(urls.first(), 0);
            return;
        }
    }
    setCurrentUrl(url, 0);
}

void DetailSpaceWidget::setCurrentUrl(const QUrl &url, int widgetFilter)
{
    detailSpaceUrl = url;

    if (!isVisible())
        return;

    removeControls();
    detailView->setUrl(url, widgetFilter);
}

QUrl DetailSpaceWidget::currentUrl() const
{
    return detailSpaceUrl;
}

int DetailSpaceWidget::detailWidth() const
{
    return preferredWidth > 0 ? preferredWidth : defaultDetailWidth();
}

bool DetailSpaceWidget::insterExpandControl(const int &index, QWidget *widget)
{
    return detailView->insertCustomControl(index, widget);
}

void DetailSpaceWidget::removeControls()
{
    detailView->removeControl();
}

void DetailSpaceWidget::prepareForAnimation(bool show)
{
    animationInProgress = true;
    resizing = false;
    updateCursorShape(false);

    if (show) {
        setFixedWidth(0);
        setMinimumWidth(0);
        setMaximumWidth(detailWidth());
    } else {
        setPreferredWidth(width(), false);
        setMinimumWidth(0);
        setMaximumWidth(width());
    }
}

void DetailSpaceWidget::handleAnimationFinished(bool show)
{
    animationInProgress = false;
    if (!show)
        setVisible(false);

    applyPreferredWidth();
}

void DetailSpaceWidget::resetWidthRange()
{
    setMinimumWidth(minDetailWidth());
    setMaximumWidth(maxDetailWidth());
}

bool DetailSpaceWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == detailView) {
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            return handleMousePress(static_cast<QMouseEvent *>(event), mapFromChild(static_cast<QMouseEvent *>(event)));
        case QEvent::MouseMove:
            if (handleMouseMove(static_cast<QMouseEvent *>(event), mapFromChild(static_cast<QMouseEvent *>(event))))
                return true;
            break;
        case QEvent::MouseButtonRelease:
            return handleMouseRelease(static_cast<QMouseEvent *>(event), mapFromChild(static_cast<QMouseEvent *>(event)));
        case QEvent::Leave:
            if (!resizing && !animationInProgress)
                updateCursorShape(false);
            break;
        default:
            break;
        }
    }

    return AbstractFrame::eventFilter(watched, event);
}

void DetailSpaceWidget::mousePressEvent(QMouseEvent *event)
{
    if (handleMousePress(event, event->pos()))
        return;

    AbstractFrame::mousePressEvent(event);
}

void DetailSpaceWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (handleMouseMove(event, event->pos()))
        return;

    AbstractFrame::mouseMoveEvent(event);
}

void DetailSpaceWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (handleMouseRelease(event, event->pos()))
        return;

    AbstractFrame::mouseReleaseEvent(event);
}

void DetailSpaceWidget::leaveEvent(QEvent *event)
{
    if (!resizing && !animationInProgress)
        updateCursorShape(false);

    AbstractFrame::leaveEvent(event);
}

bool DetailSpaceWidget::handleMousePress(QMouseEvent *event, const QPoint &localPos)
{
    if (!event || animationInProgress)
        return false;

    if (event->button() != Qt::LeftButton)
        return false;

    if (!isOnResizeArea(localPos))
        return false;

    resizing = true;
    resizeStartGlobalX = eventGlobalX(event);
    resizeStartWidth = width();
    updateCursorShape(true);
    event->accept();
    return true;
}

bool DetailSpaceWidget::handleMouseMove(QMouseEvent *event, const QPoint &localPos)
{
    if (!event)
        return false;

    if (resizing) {
        int newWidth = clampWidth(resizeStartWidth + (resizeStartGlobalX - eventGlobalX(event)));
        if (newWidth != width()) {
            setFixedWidth(newWidth);
            resetWidthRange();
        }
        event->accept();
        return true;
    }

    if (animationInProgress)
        return false;

    updateCursorShape(isOnResizeArea(localPos));
    return false;
}

bool DetailSpaceWidget::handleMouseRelease(QMouseEvent *event, const QPoint &localPos)
{
    if (!event || event->button() != Qt::LeftButton)
        return false;

    if (!resizing)
        return false;

    resizing = false;

    int newWidth = clampWidth(width());
    int delta = resizeStartWidth - newWidth;
    setFixedWidth(newWidth);
    resetWidthRange();
    setPreferredWidth(newWidth, true);

    if (delta != 0)
        notifyWorkspaceWidthDelta(delta);

    updateCursorShape(isOnResizeArea(localPos));
    event->accept();
    return true;
}

void DetailSpaceWidget::initializeUi()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::ColorRole::Base);

    QHBoxLayout *rvLayout = new QHBoxLayout(this);
    rvLayout->setContentsMargins(0, 0, 0, 0);
    detailView = new DetailView(this);
    detailView->setMouseTracking(true);
    detailView->installEventFilter(this);
    rvLayout->addWidget(detailView, 1);
    setLayout(rvLayout);
}

void DetailSpaceWidget::initConnect()
{
}

void DetailSpaceWidget::applyPreferredWidth()
{
    preferredWidth = clampWidth(preferredWidth > 0 ? preferredWidth : defaultDetailWidth());
    setFixedWidth(preferredWidth);
    resetWidthRange();
}

void DetailSpaceWidget::setPreferredWidth(int width, bool fromUser)
{
    preferredWidth = clampWidth(width);
    if (fromUser)
        userResized = true;
}

int DetailSpaceWidget::defaultDetailWidth() const
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    return DSizeModeHelper::element(260, 290);
#else
    return 290;
#endif
}

int DetailSpaceWidget::minDetailWidth() const
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    return DSizeModeHelper::element(220, 240);
#else
    return 240;
#endif
}

int DetailSpaceWidget::maxDetailWidth() const
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    return DSizeModeHelper::element(460, 520);
#else
    return 520;
#endif
}

int DetailSpaceWidget::clampWidth(int width) const
{
    return std::clamp(width, minDetailWidth(), maxDetailWidth());
}

bool DetailSpaceWidget::isOnResizeArea(const QPoint &pos) const
{
    return pos.x() >= 0 && pos.x() <= kResizeHandleWidth;
}

void DetailSpaceWidget::updateCursorShape(bool inResizeArea)
{
    bool useResizeCursor = (resizing || inResizeArea) && !animationInProgress;
    if (useResizeCursor) {
        setCursor(Qt::SizeHorCursor);
        if (detailView)
            detailView->setCursor(Qt::SizeHorCursor);
    } else {
        unsetCursor();
        if (detailView)
            detailView->unsetCursor();
    }
}

void DetailSpaceWidget::notifyWorkspaceWidthDelta(int delta)
{
    quint64 winId = DetailSpaceHelper::findWindowIdByDetailSpace(this);
    if (!winId)
        return;

    dpfSlotChannel->push("dfmplugin_workspace", "slot_View_AboutToChangeViewWidth", winId, delta);
}

QPoint DetailSpaceWidget::mapFromChild(const QMouseEvent *event) const
{
    if (!detailView || !event)
        return {};

    return detailView->mapTo(this, event->pos());
}

int DetailSpaceWidget::eventGlobalX(const QMouseEvent *event) const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return static_cast<int>(event->globalPosition().x());
#else
    return event->globalPos().x();
#endif
}
*** End of File
