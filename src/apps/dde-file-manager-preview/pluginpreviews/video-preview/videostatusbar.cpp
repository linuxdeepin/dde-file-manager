// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videostatusbar.h"
#include "videopreview.h"

#include <DGuiApplicationHelper>
#include <DIconButton>

#include <QApplication>
#include <QEvent>
#include <QIcon>
#include <QPainter>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;

namespace {
constexpr int kControlButtonIconSize { 16 };

QIcon createColoredIcon(const QString &iconName, const QColor &color, const QSize &size, const QWidget *widget)
{
    const qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QPixmap pixmap = QIcon::fromTheme(iconName).pixmap(size, dpr);
    if (pixmap.isNull())
        return QIcon();

    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);

    return QIcon(pixmap);
}

QColor controlIconColor(bool hovered)
{
    const bool darkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    QColor color = darkTheme ? QColor(Qt::white) : QColor(Qt::black);
    if (!hovered)
        color.setAlphaF(0.7);
    return color;
}
}

VideoStatusBar::VideoStatusBar(VideoPreview *preview)
    : QWidget(nullptr), p(preview), slider(new DSlider(Qt::Horizontal, this)), timeLabel(new QLabel(this)), sliderIsPressed(false)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    controlButton = new DIconButton(this);
    controlButton->setIconSize({ kControlButtonIconSize, kControlButtonIconSize });
    controlButton->installEventFilter(this);
    updateControlButtonIcon();

    slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    slider->setMinimum(0);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(0, 0, 20, 0);
    layout->addWidget(controlButton);
    layout->addWidget(slider);
    layout->addWidget(timeLabel);

    connect(controlButton, &DIconButton::clicked, this, [this] {
        // 由于调用了setBackendProperty("keep-open", "yes")
        // 导致视频播放状态不对（要暂停到最后一帧，所以视频播放完毕后状态还是暂停）
        // 如果是暂停状态，调用pause一定会播放，之后再调用play也没有影响
        // 反之，如果是停止状态，调用pause无反应，之后再调用play肯定会播放
        p->pause();
        p->playerWidget->engine().play();
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this] {
                updateControlButtonIcon();
            });

    connect(p, &VideoPreview::sigPlayState, this, [this] {
        controlButtonShowsPause = p->playerWidget->engine().state() == dmr::PlayerEngine::Playing;
        updateControlButtonIcon();
    });

    connect(slider, &DSlider::valueChanged, this, [this] {
        p->playerWidget->engine().seekAbsolute(slider->value());
    });

    connect(slider, &DSlider::sliderPressed, this, [this] {
        sliderIsPressed = true;
    });

    connect(slider, &DSlider::sliderReleased, this, [this] {
        sliderIsPressed = false;
    });

    connect(p, &VideoPreview::elapsedChanged, this, [this] {
        if (!sliderIsPressed) {
            QSignalBlocker blocker(slider);
            Q_UNUSED(blocker)
            slider->setValue(static_cast<int>(p->playerWidget->engine().elapsed()));
        }
        timeLabel->setText(dmr::utils::Time2str(p->playerWidget->engine().elapsed()));
    });
}

bool VideoStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == controlButton) {
        switch (event->type()) {
        case QEvent::Enter:
            controlButtonHovered = true;
            updateControlButtonIcon();
            break;
        case QEvent::Leave:
            controlButtonHovered = false;
            updateControlButtonIcon();
            break;
        case QEvent::StyleChange:
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
        case QEvent::ScreenChangeInternal:
        case QEvent::DevicePixelRatioChange:
            updateControlButtonIcon();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void VideoStatusBar::updateControlButtonIcon()
{
    if (!controlButton)
        return;

    const QString iconName = controlButtonShowsPause ? QStringLiteral("dfm_pause")
                                                     : QStringLiteral("dfm_start");
    const QSize iconSize(kControlButtonIconSize, kControlButtonIconSize);
    controlButton->setIconSize(iconSize);
    controlButton->setIcon(createColoredIcon(iconName, controlIconColor(controlButtonHovered), iconSize, controlButton));
}
