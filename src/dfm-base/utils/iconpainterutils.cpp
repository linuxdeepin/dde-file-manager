// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconpainterutils.h"

#include <dfm-base/utils/iconutils.h>
#include <dfm-base/utils/iconcachemanager.h>

#include <QApplication>
#include <QPainterPath>

#include <cmath>

#define CEIL(x) (static_cast<int>(std::ceil(x)))

DFMBASE_USE_NAMESPACE

/*!
 * \brief visualAlignment 文本对齐辅助函数
 */
Qt::Alignment IconPainterUtils::visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
{
    if (!(alignment & Qt::AlignHorizontal_Mask))
        alignment |= Qt::AlignLeft;
    if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
        if (direction == Qt::RightToLeft)
            alignment ^= (Qt::AlignLeft | Qt::AlignRight);
        alignment |= Qt::AlignAbsolute;
    }
    return alignment;
}

/*!
 * \brief getIconPixmap 基于 iconName 获取 icon 的 pixmap（带 QPixmapCache 缓存）
 */
QPixmap IconPainterUtils::getIconPixmap(const QString &iconName, const QSize &size,
                                        qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    if (iconName.isEmpty() || size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    QPixmap px = IconCacheManager::getPixmap(iconName, size, pixelRatio, mode, state);
    px.setDevicePixelRatio(pixelRatio);
    return px;
}

/*!
 * \brief getIconPixmap 基于 QIcon 获取 icon 的 pixmap（缩略图/兼容路径）
 */
QPixmap IconPainterUtils::getIconPixmap(const QIcon &icon, const QSize &size,
                                        qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    // 不用绘制空白的图片，使用unknown
    if (icon.isNull())
        return getIconPixmap("unknown", size, pixelRatio, mode, state);

    // 确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    auto px = icon.pixmap(size * pixelRatio, mode, state);
    if (px.isNull() || px.size().isEmpty())
        return QPixmap();

    px.setDevicePixelRatio(pixelRatio);

    return px;
}

/*!
 * \brief paintIcon 绘制指定区域内 icon 的 pixmap
 *
 * \return 成功绘制返回 painted rect；失败返回 std::nullopt
 */
std::optional<QRect> IconPainterUtils::paintIcon(QPainter *painter, const QIcon &icon, const PaintIconOpts &opts)
{
    // Copy of QStyle::alignedRect
    Qt::Alignment alignment { visualAlignment(painter->layoutDirection(), opts.alignment) };
    const qreal pixelRatio = painter->device()->devicePixelRatioF();

    // 主题图标：使用 iconName 走 QPixmapCache 缓存路径
    QPixmap px;
    if (!opts.isThumb && !opts.iconName.isEmpty()) {
        px = getIconPixmap(opts.iconName, opts.rect.size().toSize(), pixelRatio, opts.mode, opts.state);
    } else {
        px = getIconPixmap(icon, opts.rect.size().toSize(), pixelRatio, opts.mode, opts.state);
    }

    // 缩略图缩放到指定的size，绘制不出来就直接返回，绘制fileicon
    if (px.isNull() && opts.isThumb)
        return std::nullopt;

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

    // Task: 337513 — 缩略图模式下绘制带阴影背景
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

        QRectF imageRect { x, y, w, h };

        // Calculate aspect ratio for proportional adjustments
        qreal radio = w / h;
        // Avoid collapsing very narrow/tall thumbnails (e.g. 99x1026) to 0px after inset.
        // Calculate inset proportionally to maintain aspect ratio
        int insetX, insetY;
        if (radio > 1.0) {
            // Wide image: insetY should be proportionally smaller
            insetX = qMin<qreal>(iconStyle.shadowRange, qMax<qreal>(0.0, (imageRect.width() - 1.0) / 2.0));
            insetY = qRound(insetX / radio);
        } else if (radio < 1.0 && radio > 0) {
            // Tall image: insetX should be proportionally smaller
            insetY = qMin<qreal>(iconStyle.shadowRange, qMax<qreal>(0.0, (imageRect.height() - 1.0) / 2.0));
            insetX = qRound(insetY * radio);
        } else {
            // Square or invalid ratio
            insetX = qMin<qreal>(iconStyle.shadowRange, qMax<qreal>(0.0, (imageRect.width() - 1.0) / 2.0));
            insetY = insetX;
        }

        imageRect.adjust(insetX, insetY, -insetX, -insetY);

        // 绘制带有阴影的背景：
        // backgroundRect = imageRect 外扩 stroke，作为白色底板区域
        // shadowRect     = backgroundRect 再外扩 shadowRange，用于容纳阴影扩散
        auto stroke { iconStyle.stroke };
        auto shadowRange { iconStyle.shadowRange };
        QRectF backgroundRect { imageRect };
        backgroundRect.adjust(-stroke, -stroke, stroke, stroke);
        const auto &originPixmap { IconUtils::renderIconBackground(backgroundRect.size(), iconStyle) };
        // addShadowToPixmap 会在四周各扩展 shadowRange 像素来容纳阴影
        const auto &shadowPixmap { IconUtils::addShadowToPixmap(originPixmap, iconStyle.shadowOffset, shadowRange, 0.2) };
        // 绘制目标区域需外扩 shadowRange，使阴影像素完整显示
        QRectF shadowRect { backgroundRect };
        shadowRect.adjust(-shadowRange, -shadowRange, shadowRange, shadowRange);
        painter->drawPixmap(shadowRect, shadowPixmap, QRectF());

        QPainterPath clipPath;
        auto radius { iconStyle.radius - iconStyle.stroke };
        clipPath.addRoundedRect(imageRect, radius, radius);
        painter->setClipPath(clipPath);
        painter->drawPixmap(imageRect, px, QRectF());
        painter->restore();

        return backgroundRect.toRect();
    }

    // 使用QRectF和drawPixmap的重载版本来正确处理缩放
    QRectF targetRect(x, y, w, h);
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawPixmap(targetRect, px, px.rect());
    painter->restore();
    return targetRect.toRect();
}

/*!
 * \brief isThumbnailIcon 判断文件是否已生成缩略图
 *
 * 排除 AppImage / UAB 等不应展示缩略图底板的类型（与 v20 shouldSkipThumbnailFrame 行为一致）。
 */
bool IconPainterUtils::isThumbnailIcon(const FileInfoPointer &info)
{
    if (!info)
        return false;

    // appimage / uab 等类型不展示缩略图底板（与 v20 shouldSkipThumbnailFrame 行为一致）
    if (IconUtils::shouldSkipThumbnailFrame(info->nameOf(NameInfoType::kMimeTypeName)))
        return false;

    const auto &attribute { info->extendAttributes(ExtInfoType::kFileThumbnail) };
    if (attribute.isValid() && !attribute.value<QIcon>().isNull())
        return true;

    return false;
}
