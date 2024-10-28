// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/viewoptionsbutton_p.h"
#include "views/viewoptionsbutton.h"
#include "views/viewoptionswidget.h"

#include <dfm-base/base/application/application.h>

#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QTimer>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;
using DFMBASE_NAMESPACE::Global::ViewMode;

ViewOptionsButtonPrivate::ViewOptionsButtonPrivate(ViewOptionsButton *qq)
    : QObject(qq), q(qq)
{
    initConnect();
}

ViewOptionsButtonPrivate::~ViewOptionsButtonPrivate()
{
}

void ViewOptionsButtonPrivate::initConnect()
{
    connect(q, &DToolButton::clicked, this, [this](bool checked) {
        const QPoint popupPos = q->parentWidget()->mapToGlobal(q->geometry().bottomLeft());
        if (!viewOptionsWidget) {
            viewOptionsWidget = new ViewOptionsWidget(q);
            connect(viewOptionsWidget, &ViewOptionsWidget::displayPreviewVisibleChanged, q, &ViewOptionsButton::displayPreviewVisibleChanged);
        }
        viewOptionsWidget->exec(popupPos, viewMode, fileUrl);
    });
}

bool ViewOptionsButtonPrivate::popupVisible() const
{
    bool visible = false;
    if (viewOptionsWidget && viewOptionsWidget->isVisible())
        visible = true;
    return visible;
}

ViewOptionsButton::ViewOptionsButton(QWidget *parent)
    : DToolButton(parent), d(new ViewOptionsButtonPrivate(this))
{
}

void ViewOptionsButton::switchMode(ViewMode mode, const QUrl &url)
{
    d->viewMode = mode;
    d->fileUrl = url;
}

void ViewOptionsButton::setVisible(bool visible)
{
    DToolButton::setVisible(visible);
    if (!Application::instance()->appAttribute(Application::kShowedDisplayPreview).toBool())
        return;
    QTimer::singleShot(200, [this, visible]() {
        Q_EMIT displayPreviewVisibleChanged(visible);
    });
}

ViewOptionsButton::~ViewOptionsButton() = default;

void ViewOptionsButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOptionToolButton option;
    QToolButton::initStyleOption(&option);
    if (d->hoverFlag || d->popupVisible())
        option.state |= QStyle::State_MouseOver;
    else
        option.state &= ~QStyle::State_MouseOver;
    painter.drawComplexControl(QStyle::CC_ToolButton, option);
}

void ViewOptionsButton::mousePressEvent(QMouseEvent *event)
{
    DToolButton::mousePressEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void ViewOptionsButton::mouseMoveEvent(QMouseEvent *event)
{
    DToolButton::mouseMoveEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void ViewOptionsButton::mouseReleaseEvent(QMouseEvent *event)
{
    DToolButton::mouseReleaseEvent(event);
    update();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ViewOptionsButton::enterEvent(QEnterEvent *event)
#else
void ViewOptionsButton::enterEvent(QEvent *event)
#endif
{
    DToolButton::enterEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void ViewOptionsButton::leaveEvent(QEvent *event)
{
    DToolButton::leaveEvent(event);
    if (d->hoverFlag) {
        d->hoverFlag = false;
        update();
    }
}
