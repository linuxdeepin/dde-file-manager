// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <player_widget.h>
#include <player_engine.h>
#include <danchors.h>

#include <QLabel>

class VideoPreview;

class VideoWidget : public dmr::PlayerWidget
{
public:
    explicit VideoWidget(VideoPreview *preview);

    QSize sizeHint() const override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    VideoPreview *p;
    QLabel *title;
};

#endif // PLAYERWIDGET_H
