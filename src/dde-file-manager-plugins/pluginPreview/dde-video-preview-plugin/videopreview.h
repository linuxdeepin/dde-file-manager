// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include "dfmfilepreview.h"
#include "durl.h"
#include "videowidget.h"
#include "videostatusbar.h"

#include <playlist_model.h>

#include <QFileInfo>
#include <QPointer>

class VideoPreview : public DFM_NAMESPACE::DFMFilePreview
{
    Q_OBJECT

public:
    explicit VideoPreview(QObject *parent = nullptr);
    ~VideoPreview() override;

    bool setFileUrl(const DUrl &url) override;
    DUrl fileUrl() const override;

    QWidget *contentWidget() const override;
    QWidget *statusBarWidget() const override;

    bool showStatusBarSeparator() const override;
    Qt::Alignment statusBarWidgetAlignment() const override;

    virtual void DoneCurrent() override;

    void play() override;
    void pause() override;
    void stop() override;

private:
    QUrl videoUrl;
    QPointer<VideoWidget> playerWidget;
    dmr::MovieInfo info;
    QPointer<VideoStatusBar> statusBar;

    friend class VideoWidget;
    friend class VideoStatusBar;
};

#endif // VIDEOPREVIEW_H
