// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSICPREVIEW_H
#define MUSICPREVIEW_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QUrl>
#include <QPointer>

namespace plugin_filepreview {
class MusicMessageView;
class ToolBarFrame;
}

namespace plugin_filepreview {
class MusicPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT

public:
    explicit MusicPreview(QObject *parent = nullptr);
    ~MusicPreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;
    QWidget *statusBarWidget() const override;
    Qt::Alignment statusBarWidgetAlignment() const override;

    void play() override;
    void pause() override;
    void stop() override;
    void handleBeforDestroy() override;

    bool canPreview(const QUrl &url) const;

private:
    QUrl currentUrl;

    QPointer<MusicMessageView> musicView;
    QPointer<ToolBarFrame> statusBarFrame;
};
}
#endif   // MUSICPREVIEWPLUGIN_H
