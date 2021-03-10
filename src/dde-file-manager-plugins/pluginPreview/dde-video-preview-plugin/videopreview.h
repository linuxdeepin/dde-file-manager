/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
