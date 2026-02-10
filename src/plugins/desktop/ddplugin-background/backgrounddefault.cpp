// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgrounddefault.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"

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

    update();
}

void BackgroundDefault::paintEvent(QPaintEvent *event)
{
    if (painted > 0)
        fmInfo() << "background painted" << painted-- << screen << !pixmap.isNull();

    if (pixmap.isNull())
        return;

    qreal scale = devicePixelRatioF();
    bool isIntegerScale = qFuzzyCompare(std::round(scale), scale);

    QPainter pa(this);
    pa.setRenderHint(QPainter::SmoothPixmapTransform, !isIntegerScale);

    // 整数对齐源坐标
    QPointF srcTopLeft = QPointF(event->rect().topLeft()) * scale;
    srcTopLeft = { std::round(srcTopLeft.x()), std::round(srcTopLeft.y()) };
    QSizeF srcSize = QSizeF(event->rect().size()) * scale;

    pa.drawPixmap(
            event->rect().topLeft(),
            pixmap,
            QRectF(srcTopLeft, srcSize));

    sendPaintReport();
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
