// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MUSICPREVIEW_H
#define MUSICPREVIEW_H

#include "dfmfilepreview.h"
#include "durl.h"

#include <QPointer>

class MusicMessageView;
class ToolBarFrame;

class MusicPreview : public DFM_NAMESPACE::DFMFilePreview
{
    Q_OBJECT

public:
    explicit MusicPreview(QObject *parent = nullptr);
    ~MusicPreview() override;

    bool setFileUrl(const DUrl &url) override;
    DUrl fileUrl() const override;

    QWidget *contentWidget() const override;
    QWidget *statusBarWidget() const override;
    Qt::Alignment statusBarWidgetAlignment() const override;

    void play() override;
    void pause() override;
    void stop() override;

    bool canPreview(const DUrl &url) const;

private:
    DUrl m_url;

    QPointer<MusicMessageView> m_musicView;
    QPointer<ToolBarFrame> m_statusBarFrame;
};

#endif // MUSICPREVIEWPLUGIN_H
