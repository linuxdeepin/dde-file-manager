/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
