// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegatehelper.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/iconutils.h>

#include <QPainter>
#include <QApplication>
#include <QPainterPath>

#include <cmath>

#define CEIL(x) (static_cast<int>(std::ceil(x)))

using namespace dfmplugin_workspace;
using namespace dfmbase;

/*!
 * \brief getIconPixmap 获取icon的pixmap
 *
 * \return QPixmap icon的pixmap图片
 **/
QPixmap ItemDelegateHelper::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    // 确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    // 根据设备像素比获取合适大小的pixmap
    QSize deviceSize = size * pixelRatio;
    auto px = icon.pixmap(deviceSize, mode, state);
    px.setDevicePixelRatio(pixelRatio);

    return px;
}
/*!
 * \brief paintIcon 绘制指定区域内每一个icon的pixmap
 *
 * \return void
 **/
bool ItemDelegateHelper::paintIcon(QPainter *painter, const QIcon &icon, const PaintIconOpts &opts)
{
    // Copy of QStyle::alignedRect
    Qt::Alignment alignment = visualAlignment(painter->layoutDirection(), opts.alignment);
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, opts.rect.size().toSize(), pixelRatio, opts.mode, opts.state);
    
    // 缩略图缩放到指定的size，绘制不出来就直接返回，绘制fileicon
    if (px.isNull() && opts.isThumb)
        return false;

    // 保持图标原始比例
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    
    // 如果图标大于目标区域，等比例缩放
    if (w > opts.rect.width() || h > opts.rect.height()) {
        qreal scale = qMin(opts.rect.width() / w, opts.rect.height() / h);
        w *= scale;
        h *= scale;
    }
    
    qreal x = opts.rect.x();
    qreal y = opts.rect.y();

    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (opts.rect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += opts.rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += opts.rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (opts.rect.size().width() - w) / 2.0;

    // Task: 337513
    if (opts.viewMode == ViewMode::kIconMode && opts.isThumb) {
        painter->save();
        painter->setRenderHints(painter->renderHints() | QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);

        auto iconStyle { IconUtils::getIconStyle(opts.rect.size().toSize().width()) };
        
        // 计算可用的图像绘制区域（减去阴影和边框）
        QRectF availableRect = opts.rect;
        availableRect.adjust(iconStyle.shadowRange, iconStyle.shadowRange, -iconStyle.shadowRange, -iconStyle.shadowRange);
        availableRect.adjust(iconStyle.stroke, iconStyle.stroke, -iconStyle.stroke, -iconStyle.stroke);
        
        // 计算缩略图的最佳显示尺寸 - 如果小于可用区域则放大铺满
        qreal scaleX = availableRect.width() / (w > 0 ? w : 1);
        qreal scaleY = availableRect.height() / (h > 0 ? h : 1);
        qreal scale = qMin(scaleX, scaleY);
        
        // 如果原图小于可用区域，则等比放大；否则保持原逻辑
        if (scale > 1.0) {
            w *= scale;
            h *= scale;
            // 重新计算居中位置
            x = opts.rect.x() + (opts.rect.width() - w) / 2.0;
            y = opts.rect.y() + (opts.rect.height() - h) / 2.0;
        }
        
        QRect backgroundRect { qRound(x), qRound(y), qRound(w), qRound(h) };
        QRect imageRect { backgroundRect };

        // 绘制带有阴影的背景
        auto stroke { iconStyle.stroke };
        backgroundRect.adjust(-stroke, -stroke, stroke, stroke);
        const auto &originPixmap { IconUtils::renderIconBackground(backgroundRect.size(), iconStyle) };
        const auto &shadowPixmap { IconUtils::addShadowToPixmap(originPixmap, iconStyle.shadowOffset, iconStyle.shadowRange, 0.2) };
        painter->drawPixmap(backgroundRect, shadowPixmap);
        imageRect.adjust(iconStyle.shadowRange, iconStyle.shadowRange, -iconStyle.shadowRange, -iconStyle.shadowRange);

        QPainterPath clipPath;
        auto radius { iconStyle.radius - iconStyle.stroke };
        clipPath.addRoundedRect(imageRect, radius, radius);
        painter->setClipPath(clipPath);
        painter->drawPixmap(imageRect, px);
        painter->restore();

    } else {
        // 使用QRectF来避免舍入误差，同时保持比例
        QRectF targetRect(qRound(x), qRound(y), w, h);
        painter->drawPixmap(targetRect, px, px.rect());
    }

    return true;
}

void ItemDelegateHelper::drawBackground(const qreal &backgroundRadius, const QRectF &rect, QRectF &lastLineRect, const QBrush &backgroundBrush, QPainter *painter)
{
    const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
    QRectF backBounding = rect;
    QPainterPath path;
    if (lastLineRect.isValid()) {
        if (qAbs(rect.width() - lastLineRect.width()) < backgroundRadius * 2) {
            backBounding.setWidth(lastLineRect.width());
            backBounding.moveCenter(rect.center());
            path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
            path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
            path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
            path.lineTo(backBounding.x(), backBounding.bottom());
            path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
            lastLineRect = backBounding;
        } else if (lastLineRect.width() > rect.width()) {
            backBounding += margins;
            path.moveTo(backBounding.x() - backgroundRadius, backBounding.y() - 1);
            path.arcTo(backBounding.x() - backgroundRadius * 2, backBounding.y() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 90, -90);
            path.lineTo(backBounding.x(), backBounding.bottom() - backgroundRadius);
            path.arcTo(backBounding.x(), backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
            path.lineTo(backBounding.right() - backgroundRadius, backBounding.bottom());
            path.arcTo(backBounding.right() - backgroundRadius * 2, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
            path.lineTo(backBounding.right(), backBounding.top() + backgroundRadius);
            path.arcTo(backBounding.right(), backBounding.top() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 180, -90);
            path.closeSubpath();
            lastLineRect = rect;
        } else {
            backBounding += margins;
            path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
            path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
            path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
            path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
            path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
            path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
            lastLineRect = rect;
        }
    } else {
        lastLineRect = backBounding;
        path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
    }

    bool isAntialiasing = painter->testRenderHint(QPainter::Antialiasing);
    qreal painterOpacity = painter->opacity();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->fillPath(path, backgroundBrush);
    painter->setRenderHint(QPainter::Antialiasing, isAntialiasing);
    painter->setOpacity(painterOpacity);
}

void ItemDelegateHelper::hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

ElideTextLayout *ItemDelegateHelper::createTextLayout(const QString &name, QTextOption::WrapMode wordWrap,
                                                      qreal lineHeight, int alignmentFlag, QPainter *painter)
{
    ElideTextLayout *layout = new ElideTextLayout(name);

    layout->setAttribute(ElideTextLayout::kWrapMode, wordWrap);
    layout->setAttribute(ElideTextLayout::kLineHeight, lineHeight);
    layout->setAttribute(ElideTextLayout::kAlignment, alignmentFlag);

    if (painter) {
        layout->setAttribute(ElideTextLayout::kFont, painter->font());
        layout->setAttribute(ElideTextLayout::kTextDirection, painter->layoutDirection());
    }

    return layout;
}
