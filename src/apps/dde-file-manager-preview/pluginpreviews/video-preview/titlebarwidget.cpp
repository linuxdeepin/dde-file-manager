// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlebarwidget.h"

#include <QPainter>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QEasingCurve>

using namespace plugin_filepreview;

TitleBarWidget::TitleBarWidget(QWidget *parent)
    : QWidget(parent)
    , m_text()
    , m_autoHideTimer(nullptr)
    , m_fadeAnimation(nullptr)
    , m_opacity(1.0)
{
    fmDebug() << "Title bar widget: initializing TitleBarWidget";

    initializeComponents();
    setupAnimation();

    // Set fixed height and transparent background
    setFixedHeight(kTitleBarHeight);
    setAttribute(Qt::WA_TranslucentBackground);

    // Enable mouse tracking for hover events
    setMouseTracking(true);

    fmDebug() << "Title bar widget: TitleBarWidget initialization completed";
}

TitleBarWidget::~TitleBarWidget()
{
    fmDebug() << "Title bar widget: destroying TitleBarWidget";

    if (m_autoHideTimer) {
        m_autoHideTimer->stop();
        m_autoHideTimer->deleteLater();
    }

    if (m_fadeAnimation) {
        m_fadeAnimation->stop();
        m_fadeAnimation->deleteLater();
    }
}

void TitleBarWidget::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        update(); // Trigger repaint
        fmDebug() << "Title bar widget: text updated to:" << text;
    }
}

QString TitleBarWidget::text() const
{
    return m_text;
}

void TitleBarWidget::startAutoHideTimer()
{
    if (!m_autoHideTimer) {
        fmWarning() << "Title bar widget: auto-hide timer not initialized";
        return;
    }

    fmDebug() << "Title bar widget: starting auto-hide timer";
    m_autoHideTimer->start(kAutoHideDelay);
}

void TitleBarWidget::stopAutoHideTimer()
{
    if (m_autoHideTimer && m_autoHideTimer->isActive()) {
        fmDebug() << "Title bar widget: stopping auto-hide timer";
        m_autoHideTimer->stop();
    }
}

void TitleBarWidget::showAnimated()
{
    if (!m_fadeAnimation) {
        fmWarning() << "Title bar widget: fade animation not initialized";
        return;
    }

    fmDebug() << "Title bar widget: starting show animation";

    // Stop any running animation
    if (m_fadeAnimation->state() == QAbstractAnimation::Running) {
        m_fadeAnimation->stop();
    }

    // Show widget and animate to full opacity
    setVisible(true);
    m_fadeAnimation->setStartValue(m_opacity);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->start();
}

void TitleBarWidget::hideAnimated()
{
    if (!m_fadeAnimation) {
        fmWarning() << "Title bar widget: fade animation not initialized";
        return;
    }

    fmDebug() << "Title bar widget: starting hide animation";

    // Stop any running animation
    if (m_fadeAnimation->state() == QAbstractAnimation::Running) {
        m_fadeAnimation->stop();
    }

    // Animate to transparent
    m_fadeAnimation->setStartValue(m_opacity);
    m_fadeAnimation->setEndValue(0.0);
    m_fadeAnimation->start();
}

qreal TitleBarWidget::opacity() const
{
    return m_opacity;
}

void TitleBarWidget::setOpacity(qreal opacity)
{
    if (qFuzzyCompare(m_opacity, opacity)) {
        return;
    }

    m_opacity = qBound(0.0, opacity, 1.0);
    update(); // Trigger repaint with new opacity
}

void TitleBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Create gradient background (30% black to transparent, top to bottom)
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0.0, QColor(0, 0, 0, static_cast<int>(76 * m_opacity))); // 30% of 255 = 76
    gradient.setColorAt(1.0, QColor(0, 0, 0, 0)); // Fully transparent

    // Fill the background with gradient
    painter.fillRect(rect(), QBrush(gradient));

    // Draw centered text if available
    if (!m_text.isEmpty()) {
        painter.setPen(QColor(255, 255, 255, static_cast<int>(255 * m_opacity))); // White text with opacity

        // Use default font but ensure it's readable
        QFont font = painter.font();
        painter.setFont(font);

        QFontMetrics fontMetrics(font);
        QRect textRect = fontMetrics.boundingRect(m_text);

        // Calculate centered position
        int x = qMax((width() - textRect.width()) / 2, 40);
        int y = (height() + fontMetrics.ascent()) / 2;

        // Draw text with eliding if too long
        QString elidedText = fontMetrics.elidedText(m_text, Qt::ElideMiddle, width() - 80); // 40px margin on each side
        painter.drawText(x, y, elidedText);
    }
}

void TitleBarWidget::resizeEvent(QResizeEvent *event)
{
    fmDebug() << "Title bar widget: resizing to" << event->size();

    QWidget::resizeEvent(event);
    update(); // Repaint with new size
}

void TitleBarWidget::initializeComponents()
{
    // Initialize auto-hide timer
    m_autoHideTimer = new QTimer(this);
    m_autoHideTimer->setSingleShot(true);
    connect(m_autoHideTimer, &QTimer::timeout, this, &TitleBarWidget::hideAnimated);

    fmDebug() << "Title bar widget: components initialized";
}

void TitleBarWidget::setupAnimation()
{
    // Initialize fade animation
    m_fadeAnimation = new QPropertyAnimation(this, "opacity", this);
    m_fadeAnimation->setDuration(kFadeAnimationDuration);
    m_fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    fmDebug() << "Title bar widget: animation setup completed";
}
