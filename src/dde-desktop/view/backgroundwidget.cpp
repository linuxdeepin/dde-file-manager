/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#include "backgroundwidget.h"
#include "canvasgridview.h"
#include "accessibility/ac-desktop-define.h"

#include <QPaintEvent>
#include <QBackingStore>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformbackingstore.h>
#include <QPaintDevice>
#include <QPainter>
#include <QImage>
#include <qdebug.h>

BackgroundWidget::BackgroundWidget(QWidget *parent)
    : QWidget(parent)
{
    AC_SET_OBJECT_NAME( this, AC_SCREEN_BACKGROUND);
    AC_SET_ACCESSIBLE_NAME( this, AC_SCREEN_BACKGROUND);
}

BackgroundWidget::~BackgroundWidget()
{
    if (m_view.get() != nullptr){
        m_view->setParent(nullptr);
        m_view = nullptr;
    }
}

void BackgroundWidget::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    m_noScalePixmap = pixmap;
    m_noScalePixmap.setDevicePixelRatio(1);
    update();
}

void BackgroundWidget::paintEvent(QPaintEvent *event)
{
    qreal scale = devicePixelRatioF();
    if (scale > 1.0 && event->rect() == rect()) {
        if (backingStore()->handle()->paintDevice()->devType() != QInternal::Image) {
            return;
        }

        QImage *image = static_cast<QImage *>(backingStore()->handle()->paintDevice());
        QPainter pa(image);
        pa.drawPixmap(0, 0, m_noScalePixmap);
        return;
    }

    QPainter pa(this);
    pa.drawPixmap(event->rect().topLeft(), m_pixmap, QRectF(QPointF(event->rect().topLeft()) * scale, QSizeF(event->rect().size()) * scale));
}

void BackgroundWidget::setView(const QSharedPointer<CanvasGridView> &v)
{
    if (v.get() != nullptr)
        v->setParent(this);
    m_view = v;
}

void BackgroundWidget::setAccessableInfo(const QString& info)
{
    if(info.isEmpty())
        return;

    AC_SET_OBJECT_NAME( this, info);
    AC_SET_ACCESSIBLE_NAME( this, info);
}
