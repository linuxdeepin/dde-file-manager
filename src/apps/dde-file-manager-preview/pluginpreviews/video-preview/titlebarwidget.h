// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include "preview_plugin_global.h"

#include <QWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEnterEvent>
#include <QPainter>
#include <QLinearGradient>
#include <QFontMetrics>

namespace plugin_filepreview {

/**
 * @brief Custom title bar widget with gradient background and auto-hide functionality
 * 
 * Features:
 * - Gradient background from 30% black (top) to transparent (bottom)
 * - Auto-hide after 3 seconds
 * - Fade in/out animation (1 second duration)
 * - Mouse hover interaction
 * - Centered text display
 */
class TitleBarWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit TitleBarWidget(QWidget *parent = nullptr);
    virtual ~TitleBarWidget();

    /**
     * @brief Set the title text to display
     * @param text The title text
     */
    void setText(const QString &text);

    /**
     * @brief Get current title text
     * @return Current title text
     */
    QString text() const;

    /**
     * @brief Start the auto-hide timer (3 seconds delay)
     */
    void startAutoHideTimer();

    /**
     * @brief Stop the auto-hide timer
     */
    void stopAutoHideTimer();

    /**
     * @brief Show the title bar with fade-in animation
     */
    void showAnimated();

    /**
     * @brief Hide the title bar with fade-out animation
     */
    void hideAnimated();

    /**
     * @brief Get current opacity value
     * @return Opacity value (0.0 - 1.0)
     */
    qreal opacity() const;

    /**
     * @brief Set opacity value for animation
     * @param opacity Opacity value (0.0 - 1.0)
     */
    void setOpacity(qreal opacity);

protected:
    /**
     * @brief Paint the gradient background and centered text
     * @param event Paint event
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief Handle resize event - update geometry
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief Initialize the widget components
     */
    void initializeComponents();

    /**
     * @brief Setup fade animation
     */
    void setupAnimation();

    QString m_text;                      ///< Title text to display
    QTimer *m_autoHideTimer;             ///< Timer for auto-hide functionality
    QPropertyAnimation *m_fadeAnimation; ///< Animation for fade in/out effects
    qreal m_opacity;                     ///< Current opacity value
    
    static constexpr int kTitleBarHeight = 40;     ///< Fixed height of title bar
    static constexpr int kAutoHideDelay = 3000;    ///< Auto-hide delay in milliseconds
    static constexpr int kFadeAnimationDuration = 500; ///< Fade animation duration in milliseconds
};

}

#endif // TITLEBARWIDGET_H
