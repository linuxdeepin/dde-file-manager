// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOSTATUSBAR_H
#define VIDEOSTATUSBAR_H

#include <QWidget>
#include <QLabel>

#include <DSlider>
#include <danchors.h>

class VideoPreview;

DWIDGET_BEGIN_NAMESPACE
class DSlider;
DWIDGET_END_NAMESPACE

class VideoStatusBar : public QWidget
{
public:
    explicit VideoStatusBar(VideoPreview *preview);

    VideoPreview *p;
    DTK_WIDGET_NAMESPACE::DSlider *slider;
    QLabel *timeLabel;
    bool sliderIsPressed;
};

#endif // VIDEOSTATUSBAR_H
