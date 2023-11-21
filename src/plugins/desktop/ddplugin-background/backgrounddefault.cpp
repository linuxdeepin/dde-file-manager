// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgrounddefault.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>
#include <dfm-base/dfm_global_defines.h>

#include <QPaintEvent>
#include <QBackingStore>
#include <QPainter>
#include <QFileInfo>
#include <QImageReader>
#include <QPaintDevice>
#include <QDateTime>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformbackingstore.h>

DFMBASE_USE_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE

BackgroundDefault::BackgroundDefault(const QString &screenName, QWidget *parent)
    : QWidget(parent), screen(screenName)

{
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void BackgroundDefault::setPixmap(const QPixmap &pix)
{
    pixmap = pix;
    noScalePixmap = pix;
    noScalePixmap.setDevicePixelRatio(1);

    update();
}

void BackgroundDefault::paintEvent(QPaintEvent *event)
{
    if (painted > 0)
        fmInfo() << "background painted" << painted-- << screen << !pixmap.isNull();

    if (pixmap.isNull())
        return;

    qreal scale = devicePixelRatioF();
    if (scale > 1.0 && event->rect() == rect()) {
        if (backingStore()->handle()->paintDevice()->devType() != QInternal::Image)
            return;
        QImage *image = static_cast<QImage *>(backingStore()->handle()->paintDevice());
        QPainter pa(image);
        pa.drawPixmap(0, 0, noScalePixmap);
        sendPaintReport();
        return;
    }

    QPainter pa(this);
    pa.drawPixmap(event->rect().topLeft(), pixmap, QRectF(QPointF(event->rect().topLeft()) * scale, QSizeF(event->rect().size()) * scale));
    sendPaintReport();
    return;
}

void BackgroundDefault::sendPaintReport()
{
    static bool reportedPaint { false };
    if (Q_LIKELY(reportedPaint))
        return;

    QVariant data = QDateTime::currentDateTime().toString();
    dpfSignalDispatcher->publish("ddplugin_background", "signal_ReportLog_BackgroundPaint",
                                 QString(DFMGLOBAL_NAMESPACE::DataPersistence::kDesktopDrawWallpaperTime),
                                 data);
    reportedPaint = true;
}
