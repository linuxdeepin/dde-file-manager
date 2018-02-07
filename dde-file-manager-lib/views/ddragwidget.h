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

#ifndef DDRAG_H
#define DDRAG_H

#include <QDrag>
#include <QWidget>
#include <QStyleOptionGraphicsItem>
#include <QDragMoveEvent>
#include <QDebug>
#include <QMimeData>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QPainter>
#include <QPixmap>

class PixmapWidget : public QWidget
{
public:
    PixmapWidget();
    void setPixmap(const QPixmap &pixmap);
protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void paintEvent(QPaintEvent *e);
    QPixmap m_pixmap;
};

class DDragWidget : public QDrag
{
    Q_OBJECT
public:
    DDragWidget(QObject * parent);
    ~DDragWidget();
    void startDrag();
    void setPixmap(const QPixmap & pixmap);
    void setHotSpot(const QPoint &hotspot);
    bool eventFilter(QObject *obj, QEvent *e);
private:
    PixmapWidget * m_widget;
    QTimer * m_timer;
    QPoint m_hotspot;
public slots:
    void timerEvent();
};

#endif // DDRAG_H
