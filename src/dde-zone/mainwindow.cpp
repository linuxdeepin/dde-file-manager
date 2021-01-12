/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "mainwindow.h"
#include "pushbuttonlist.h"
#include "hotzone.h"
#include <QScreen>
#include <QGuiApplication>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QPainter>
#include <dgiosettings.h>
#include "desktopinfo.h"

ZoneMainWindow::ZoneMainWindow(QWidget *parent)
    : QWidget(parent)
    , m_videoWidget(nullptr)
{
    m_dbusZoneInter = new ZoneInterface("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone", QDBusConnection::sessionBus(), this);

    // let the app start without system animation
//    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);

    if (!DesktopInfo().waylandDectected()) {
        setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(Qt::WindowStaysOnTopHint);
    }


    // let background be transparent
    setAttribute(Qt::WA_TranslucentBackground, true);

    // catch the screen that mouse is in
    QList<QScreen *> screenList = QGuiApplication::screens();
    for (int i = 0; i < screenList.length(); i++) {
        QRect screen = screenList[i]->geometry();
        if (screen.contains(QCursor::pos())) {
            // set the size and position of this app. Enlarge 30px to height to avoid fade-zone of mouseEvent.
            this->setGeometry(screen.x(), screen.y() - MAIN_ITEM_TOP_MARGIN, screen.width(), screen.height() + MAIN_ITEM_TOP_MARGIN);
            break;
        }
    }

    // set the background
    QWidget *back = new QWidget(this);
    back->setGeometry(0, MAIN_ITEM_TOP_MARGIN, this->width(), this->height() - MAIN_ITEM_TOP_MARGIN);

    // check demo video gsettings value
    DGioSettings gsetting("com.deepin.dde.desktop", "/com/deepin/dde/desktop/");
    if (gsetting.keys().contains("enable-hotzone-video") && gsetting.value("enable-hotzone-video").toBool()) {
        m_videoWidget = new DVideoWidget(this);
        m_videoWidget->setSourceVideoPixelRatio(devicePixelRatioF());
        QTimer::singleShot(1000, this, &ZoneMainWindow::onDemoVideo);
    }

    // init corresponding QList for addButtons()
    m_ButtonNames << tr("Fast Screen Off") << tr("All Windows") << tr("Launcher") << tr("Desktop") << tr("None");
    m_ActionStrs << FAST_SCREEN_OFF << ALL_WINDOWS_STR << LAUNCHER_STR << SHOW_DESKTOP_STR << NONE_STR;

    QStringList topRightNames = QStringList() << tr("Close Window") << m_ButtonNames;
    QStringList topRightActionStr = QStringList() << CLOSE_MAX_WINDOW_STR << m_ActionStrs;

    // load 4 corners
    HotZone *hotzone1 = new HotZone(this, false, false);
    hotzone1->addButtons(m_ButtonNames, m_ActionStrs);

    HotZone *hotzone2 = new HotZone(this, true, false);
    hotzone2->addButtons(topRightNames, topRightActionStr);

    HotZone *hotzone3 = new HotZone(this, false, true);
    hotzone3->addButtons(m_ButtonNames, m_ActionStrs);

    HotZone *hotzone4 = new HotZone(this, true, true);
    hotzone4->addButtons(m_ButtonNames, m_ActionStrs);

    m_dbusZoneInter->EnableZoneDetected(false);
}

ZoneMainWindow::~ZoneMainWindow()
{
    m_dbusZoneInter->EnableZoneDetected(true);
}

void ZoneMainWindow::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton || e->button() == Qt::MiddleButton) {
        releaseKeyboard();
        emit finished();
    }
}

void ZoneMainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        releaseKeyboard();
        emit finished();
    }
}

void ZoneMainWindow::onDemoVideo()
{
    if (!m_videoWidget) return;

    QMediaPlayer *player = new QMediaPlayer(this);

    QMediaPlaylist *list = new QMediaPlaylist(this);

    list->addMedia(QUrl("qrc:/images/Prompt.mov"));
    list->setPlaybackMode(QMediaPlaylist::Loop);

    int x = (rect().right() - 450) / 2;
    int y = (rect().bottom() - 348) / 2;
    m_videoWidget->setGeometry(x, y, 450, 348);

    m_videoWidget->setSource(player);
    player->setPlaylist(list);
    player->play();
}

void ZoneMainWindow::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter pa(this);

    pa.fillRect(rect(), QColor(0, 0, 0, 178));
}
