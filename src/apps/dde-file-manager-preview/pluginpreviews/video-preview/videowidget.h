// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include "preview_plugin_global.h"
#include "titlebarwidget.h"

#include <danchors.h>

#include <QLabel>
#include <QMouseEvent>
#include <QEnterEvent>

#include <player_widget.h>
#include <player_engine.h>

namespace plugin_filepreview {
class VideoPreview;
class VideoWidget : public dmr::PlayerWidget
{
public:
    explicit VideoWidget(VideoPreview *preview);

    QSize sizeHint() const override;

    void playFile(const QUrl &url);

    void mouseReleaseEvent(QMouseEvent *event) override;

    void enterEvent(QEnterEvent *event) override;

    void leaveEvent(QEvent *event) override;

    void showEvent(QShowEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    VideoPreview *p;
    TitleBarWidget *titleBar;
    QUrl videoUrl;
};
}
#endif   // PLAYERWIDGET_H
