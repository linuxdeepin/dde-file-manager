// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOSTATUSBAR_H
#define VIDEOSTATUSBAR_H

#include "preview_plugin_global.h"

#include <DIconButton>

#include <QWidget>
#include <QSlider>
#include <QLabel>

class QObject;
class QEvent;

namespace plugin_filepreview {
class VideoPreview;

class VideoStatusBar : public QWidget
{
public:
    explicit VideoStatusBar(VideoPreview *preview);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updateControlButtonIcon();

public:
    VideoPreview *p;
    QSlider *slider;
    QLabel *timeLabel;
    bool sliderIsPressed;

private:
    DTK_WIDGET_NAMESPACE::DIconButton *controlButton { nullptr };
    bool controlButtonHovered { false };
    bool controlButtonShowsPause { true };
};
}
#endif   // VIDEOSTATUSBAR_H
