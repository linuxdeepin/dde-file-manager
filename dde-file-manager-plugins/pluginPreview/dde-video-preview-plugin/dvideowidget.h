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

#ifndef DVIDEOWIDGET_H
#define DVIDEOWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QAbstractNativeEventFilter>
#include <QWindow>
#include <QWindowList>
#include <QDebug>

#include <mpv/client.h>

#include <xcb/xproto.h>
#include <xcb/xcb_aux.h>
#include <QX11Info>
#include <xcb/xproto.h>
#undef Bool


class DVideoWidget : public QWidget
{
    Q_OBJECT

public:
    enum State{
        Playing,
        Paused,
        Unknow
    };
    explicit DVideoWidget(QWidget *parent = 0);
    ~DVideoWidget();

    void initConnections();
    void loadFile(const QString& uri);

    qint64 duration() const;

    qint64 pos() const;

private slots:
    void onMpvEvents();

public slots:
    void seekPos(const qint64& timePos);
    void resume();
    void pause();

    void onSubwindowCreated(xcb_window_t winid);
    void onSubwindowMapped(xcb_window_t winid);

signals:
    void mpvEvents();
    void positionChanged(const qint64& timePos);
    void fileLoaded();
    void stateChanged(const State& state);

private:
    void handleMpvEvent(mpv_event *event);

    mpv_handle* m_mpv;
    qint64 m_duration;
    qint64 m_pos;
    bool m_isPaused = true;
    State m_state = Unknow;
};

class EventRelayer2: public QObject, public QAbstractNativeEventFilter{
    Q_OBJECT

public:
    explicit EventRelayer2(QWindow* src, QWindow *dest, QObject *parent = 0);
    ~EventRelayer2() {
        qApp->removeNativeEventFilter(this);
    }

    QWindow *_source, *_target;

signals:
    void subwindowCreated(xcb_window_t winid);
    void subwindowMapped(xcb_window_t winid);

protected:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *data);
};

#endif // DVIDEOWIDGET_H
