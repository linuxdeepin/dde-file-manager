// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundpreview.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

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
using namespace ddplugin_wallpapersetting;

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

BackgroundPreview::BackgroundPreview(const QString &screenName, QWidget *parent)
    : QWidget(parent), screen(screenName)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
}

void BackgroundPreview::paintEvent(QPaintEvent *event)
{
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

void BackgroundPreview::setDisplay(const QString &path)
{
    filePath = path;
    updateDisplay();
}

void BackgroundPreview::updateDisplay()
{
    QPixmap defaultImage;
    QPixmap backgroundPixmap = getPixmap(filePath, defaultImage);

    auto winMap = rootMap();
    auto *win = winMap.value(screen);
    if (win == nullptr) {
        fmCritical() << "Can not get root window for screen:" << screen;
        return;
    }

    QSize trueSize = win->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect().size();   // 使用屏幕缩放前的分辨率
    fmDebug() << "Screen" << screen << "true size:" << trueSize;

    if (backgroundPixmap.isNull()) {
        fmCritical() << "Screen" << screen << "background path" << filePath
                     << "can not read, using white fallback";
        backgroundPixmap = QPixmap(trueSize);
        backgroundPixmap.fill(Qt::white);
    }

    auto pix = backgroundPixmap.scaled(trueSize,
                                       Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation);

    if (pix.width() > trueSize.width() || pix.height() > trueSize.height()) {
        pix = pix.copy(QRect(static_cast<int>((pix.width() - trueSize.width()) / 2.0),
                             static_cast<int>((pix.height() - trueSize.height()) / 2.0),
                             trueSize.width(),
                             trueSize.height()));
    }

    fmDebug() << "Screen" << screen << "background path" << filePath << "truesize" << trueSize
              << "devicePixelRatio" << this->devicePixelRatioF() << "final pixmap size:" << pix.size();

    pix.setDevicePixelRatio(this->devicePixelRatioF());

    pixmap = pix;
    noScalePixmap = pix;
    noScalePixmap.setDevicePixelRatio(1);
    update();
}

QPixmap BackgroundPreview::getPixmap(const QString &path, const QPixmap &defalutPixmap)
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
