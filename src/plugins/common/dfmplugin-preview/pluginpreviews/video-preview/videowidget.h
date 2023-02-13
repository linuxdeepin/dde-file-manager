// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include "preview_plugin_global.h"

#include <danchors.h>

#include <QLabel>

#include <player_widget.h>
#include <player_engine.h>

namespace plugin_filepreview {
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
}
#endif   // PLAYERWIDGET_H
