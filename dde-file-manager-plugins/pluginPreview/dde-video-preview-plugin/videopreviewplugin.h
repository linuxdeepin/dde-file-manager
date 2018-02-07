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

#ifndef VIDEOPREVIEWPLUGIN_H
#define VIDEOPREVIEWPLUGIN_H

#include <QWidget>
#include <QApplication>
#include <QAbstractNativeEventFilter>
#include <QWindow>
#include <QWindowList>
#include <QDebug>

#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <QX11Info>
#include <xcb/xproto.h>
#undef Bool

#include "../../plugininterfaces/preview/previewinterface.h"

class QLabel;
class DVideoWidget;
class QSlider;
class QFrame;
class MpvProxyWidget;
class VideoPreviewPlugin : public PreviewInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PreviewInterface_iid FILE "dde-video-preview-plugin.json")
    Q_INTERFACES(PreviewInterface)

public:
    VideoPreviewPlugin(QObject *parent = 0);

    void init(const QString &uri);

    QWidget* previewWidget();

    QSize previewWidgetMinSize() const;

    bool canPreview() const;

    QWidget* toolBarItem();

    QString pluginName() const;

    QIcon pluginLogo() const;

    QString pluginDescription() const;

private:

    QString formatNumberString(const int& number) const;

    QStringList m_supportSuffixes;
    QString m_uri;
    QSlider* m_progressSlider;
    QLabel* m_durationLabel;
    DVideoWidget* m_videoWidget;
    QFrame* m_toolBarFrame;
    QStringList m_supportyMimeTypes;
//    MpvProxyWidget* m_mpvProxyWidget;

};

#endif // VIDEOPREVIEWPLUGIN_H
