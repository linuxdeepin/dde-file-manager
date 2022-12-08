/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "backgrounddefault.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include "dfm-base/dfm_desktop_defines.h"
#include "interfaces/screen/abstractscreen.h"

#include <QPaintEvent>
#include <QBackingStore>
#include <QPainter>
#include <QFileInfo>
#include <QImageReader>
#include <QPaintDevice>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformbackingstore.h>

DFMBASE_USE_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE

inline QString getScreenName(QWidget *win)
{
    return win->property(DesktopFrameProperty::kPropScreenName).toString();
}

static QMap<QString, QWidget *> rootMap()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    QMap<QString, QWidget *> ret;
    for (QWidget *win : root) {
        QString name = getScreenName(win);
        if (name.isEmpty())
            continue;
        ret.insert(name, win);
    }

    return ret;
}

BackgroundDefault::BackgroundDefault(const QString &screenName, QWidget *parent)
    : DFMBASE_NAMESPACE::AbstractBackground (screenName, parent)
{
    setAttribute(Qt::WA_TranslucentBackground,true);
}

void BackgroundDefault::paintEvent(QPaintEvent *event)
{
    if (Statics == displayMode && Zoom::Stretch == zoom()) {

        qreal scale = devicePixelRatioF();
        if (scale > 1.0 && event->rect() == rect()) {
            if (backingStore()->handle()->paintDevice()->devType() != QInternal::Image) {
                return;
            }

            QImage *image = static_cast<QImage *>(backingStore()->handle()->paintDevice());
            QPainter pa(image);
            pa.drawPixmap(0, 0, noScalePixmap);
            return;
        }

        QPainter pa(this);
        pa.drawPixmap(event->rect().topLeft(), pixmap, QRectF(QPointF(event->rect().topLeft()) * scale, QSizeF(event->rect().size()) * scale));
    }
    // 这里写其他扩展逻辑
    DFMBASE_NAMESPACE::AbstractBackground::paintEvent(event);

    if (painted > 0)
        qInfo() << "background painted" << painted-- << screen;

    return;
}

void BackgroundDefault::setMode(int mode)
{
    displayMode = static_cast<Mode>(mode);

    if (Statics != displayMode) {
        pixmap.fill(Qt::transparent);
        noScalePixmap.fill(Qt::transparent);
    }

    update();
}

void BackgroundDefault::setDisplay(const QString &path)
{
    filePath = path;

    updateDisplay();
}

void BackgroundDefault::updateDisplay()
{
    if (Statics == displayMode) {

        QPixmap defaultImage;
        QPixmap backgroundPixmap = getPixmap(filePath, defaultImage);

        auto winMap = rootMap();
        auto *win = winMap.value(screen);
        if (win == nullptr) {
            qCritical() << "can not get root " << screen;
            return;
        }

        QSize trueSize = win->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect().size(); // 使用屏幕缩放前的分辨率
        if (backgroundPixmap.isNull()) {
            qCritical() << "screen " << screen << "backfround path" << filePath
                        << "can not read!";
            backgroundPixmap = QPixmap(trueSize);
            backgroundPixmap.fill(Qt::white);
        }

        if (AbstractBackground::Stretch == displayZoom) {
            auto pix = backgroundPixmap.scaled(trueSize,
                                               Qt::KeepAspectRatioByExpanding,
                                               Qt::SmoothTransformation);

            if (pix.width() > trueSize.width() || pix.height() > trueSize.height()) {
                pix = pix.copy(QRect(static_cast<int>((pix.width() - trueSize.width()) / 2.0),
                                     static_cast<int>((pix.height() - trueSize.height()) / 2.0),
                                     trueSize.width(),
                                     trueSize.height()));
            }

            qDebug() << screen << "background path" << filePath << "truesize" << trueSize << "devicePixelRatio"
                     << this->devicePixelRatioF() << pix << "widget" << this;
            pix.setDevicePixelRatio(this->devicePixelRatioF());

            pixmap = pix;
            noScalePixmap = pix;
            noScalePixmap.setDevicePixelRatio(1);
        }

    }

    update();
}

QPixmap BackgroundDefault::getPixmap(const QString &path, const QPixmap &defalutPixmap)
{
    if (path.isEmpty())
        return defalutPixmap;

    QString currentWallpaper = path.startsWith("file:") ? QUrl(path).toLocalFile() : path;
    QPixmap backgroundPixmap(currentWallpaper);
    // fix whiteboard shows when a jpeg file with filename xxx.png
    // content formart not epual to extension
    if (backgroundPixmap.isNull()) {
        QImageReader reader(currentWallpaper);
        reader.setDecideFormatFromContent(true);
        backgroundPixmap = QPixmap::fromImage(reader.read());
    }

    return backgroundPixmap.isNull() ? defalutPixmap : backgroundPixmap;
}


