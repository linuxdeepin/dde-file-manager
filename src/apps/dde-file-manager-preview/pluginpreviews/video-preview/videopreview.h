// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include "videowidget.h"
#include "videostatusbar.h"
#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QFileInfo>
#include <QPointer>
#include <QUrl>

#include <playlist_model.h>

namespace plugin_filepreview {
class VideoPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT

public:
    explicit VideoPreview(QObject *parent = nullptr);
    ~VideoPreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;
    QWidget *statusBarWidget() const override;

    bool showStatusBarSeparator() const override;
    Qt::Alignment statusBarWidgetAlignment() const override;

    void play() override;
    void pause() override;
    void stop() override;

signals:
    void sigPlayState();

    void elapsedChanged();

private:
    QUrl videoUrl;
    QPointer<VideoWidget> playerWidget;
    dmr::MovieInfo info;
    QPointer<VideoStatusBar> statusBar;

    friend class VideoWidget;
    friend class VideoStatusBar;
};
}
#endif   // VIDEOPREVIEW_H
