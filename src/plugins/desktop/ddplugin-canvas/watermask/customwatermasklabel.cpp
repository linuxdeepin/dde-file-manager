// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customwatermasklabel.h"
#include "displayconfig.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DSysInfo>

#include <QHBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFile>
#include <QImageReader>
#include <QDebug>
#include <QtSvg/QSvgRenderer>
#include <QPainter>

using namespace ddplugin_canvas;
using namespace dfmbase;
DCORE_USE_NAMESPACE

static constexpr char kConfName[] { "org.deepin.dde.file-manager.desktop" };
static constexpr char kIsMaskEnable[] { "enableMask" };
static constexpr char kMaskLogoUri[] { "maskLogoUri" };
static constexpr char kMaskOffsetX[] { "maskOffsetX" };
static constexpr char kMaskOffsetY[] { "maskOffsetY" };
static constexpr char kMaskWidth[] { "maskWidth" };
static constexpr char kMaskHeight[] { "maskHeight" };

CustomWaterMaskLabel::CustomWaterMaskLabel(QWidget *parent)
    : QLabel(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);

    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &CustomWaterMaskLabel::onConfigChanged, Qt::DirectConnection);
}

CustomWaterMaskLabel::~CustomWaterMaskLabel()
{

}

void CustomWaterMaskLabel::loadConfig()
{
   maskEnabled = DConfigManager::instance()->value(kConfName, kIsMaskEnable, false).toBool();
   maskLogoUri = DConfigManager::instance()->value(kConfName, kMaskLogoUri, QString()).toString();

   maskOffset.setX(DConfigManager::instance()->value(kConfName, kMaskOffsetX, 0).toInt());
   maskOffset.setY(DConfigManager::instance()->value(kConfName, kMaskOffsetY, 0).toInt());

   int width = DConfigManager::instance()->value(kConfName, kMaskWidth, 0).toInt();
   int height = DConfigManager::instance()->value(kConfName, kMaskHeight, 0).toInt();

   if (width > 1920)
       maskSize.setWidth(1920);
   else if (width < 0)
       maskSize.setWidth(0);
   maskSize.setWidth(width);

   if (height > 1080)
       maskSize.setHeight(1080);
   else if (height < 0)
       maskSize.setHeight(0);
   maskSize.setHeight(height);

    if (maskLogoUri.startsWith("~/")) {
        maskLogoUri.replace(0, 1, QDir::homePath());
    }

    fmDebug() << "Config loaded - enabled:" << maskEnabled << "size:" << maskSize << "offset:" << maskOffset;

    return;
}

void CustomWaterMaskLabel::refresh()
{
    loadConfig();
    update();

    return;
}

void CustomWaterMaskLabel::setPosition()
{
    int x = systemMaskPosition.x() + maskOffset.x();
    int y = systemMaskPosition.y() + maskOffset.y();

    move(x, y);
    return;
}

void CustomWaterMaskLabel::onConfigChanged(const QString &cfg, const QString &key)
{
    if (cfg != QString(kConfName))
        return;

    fmDebug() << "Water mask config changed - key:" << key;
    refresh();
    return;
}

void CustomWaterMaskLabel::update()
{
    if (!maskEnabled) {
        fmDebug() << "Water mask disabled - hiding";
        hide();
        return;
    }

    QPixmap waterMaskPixmap = maskPixmap(maskLogoUri, maskSize, devicePixelRatioF());
    if (waterMaskPixmap.isNull()) {
        fmWarning() << "watermask pixmap NULL";
        hide();
        return;
    }

    setPixmap(waterMaskPixmap);
    setFixedSize(maskSize.width(), maskSize.height());

    setPosition();
    show();

    return;
}

void CustomWaterMaskLabel::onSystemMaskShow(const QPoint &pos)
{
    systemMaskPosition = pos;
    setPosition();

    return;
}

QPixmap CustomWaterMaskLabel::maskPixmap(const QString &uri, const QSize &size, qreal pixelRatio)
{
    QFileInfo file(uri);
    QPixmap pix;
    if (file.size() > (500 << 10)) {
        fmWarning() << "logo size exceed 500KB!";
        return pix;
    }

    fmInfo() << "custom watermask scaled start" << "uri:" << uri << " size:" << size;

    QSize scaleSize = size * pixelRatio;

    if (file.suffix() != "svg") {
        pix = QPixmap(uri);

        pix = pix.scaled(scaleSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pix.setDevicePixelRatio(pixelRatio);
        return pix;
    } else {
        pix = QPixmap(scaleSize);
        pix.fill(Qt::transparent);
        {
            QSvgRenderer svg(uri);
            QPainter painter(&pix);
            svg.render(&painter, QRect(QPoint(0,0), pix.size()));
        }
        pix.setDevicePixelRatio(pixelRatio);
    }

    fmInfo() << "custom watermask scaled finished";

    return pix;
}
