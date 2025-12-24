// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/viewoptionsbutton_p.h"
#include "views/viewoptionsbutton.h"
#include "views/viewoptionswidget.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DGuiApplicationHelper>

#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QTimer>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;
using DFMBASE_NAMESPACE::Global::ViewMode;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

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

ViewOptionsButton::~ViewOptionsButton() = default;

void ViewOptionsButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOptionToolButton option;
    QToolButton::initStyleOption(&option);

    if (d->hoverFlag || d->popupVisible()) {
        option.state |= QStyle::State_MouseOver;

        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;

        QColor hoverColor;
        if (isDown()) {
            // 按下状态 - 20%不透明度
            hoverColor = isDarkTheme ? QColor(255, 255, 255, 51)
                                     : QColor(0, 0, 0, 51);
        } else {
            // 悬浮状态 - 10%不透明度
            hoverColor = isDarkTheme ? QColor(255, 255, 255, 26)
                                     : QColor(0, 0, 0, 26);
        }

        option.palette.setBrush(QPalette::Button, hoverColor);
    } else {
        option.state &= ~QStyle::State_MouseOver;
    }

    option.rect.adjust(-1, -1, 1, 1);
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

bool ViewOptionsButton::event(QEvent *event)
{
    const auto type = event->type();
    if (type == QEvent::Show || type == QEvent::Hide) {
        if (!DConfigManager::instance()->value(kViewDConfName, kDisplayPreviewVisibleKey).toBool()) {
            fmDebug() << "Display preview is disabled in config, skipping preview visibility change";
            return DToolButton::event(event);
        }

        // Check if the window is minimized/maximized - don't emit signal in these cases
        QWidget *topWindow = window();
        if (topWindow && topWindow->isMinimized()) {
            // Window is minimized, ignore the event
            return DToolButton::event(event);
        }

        // Also check if this is caused by spontaneous event (user action vs programmatic)
        // Only react to programmatic visibility changes (detailspace show/hide)
        if (event->spontaneous()) {
            // Spontaneous event (from window system), likely minimize/maximize
            return DToolButton::event(event);
        }

        Q_EMIT displayPreviewVisibleChanged(type == QEvent::Show);
    }

    return DToolButton::event(event);
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
