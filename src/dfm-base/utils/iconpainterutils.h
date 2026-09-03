// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONPAINTERUTILS_H
#define ICONPAINTERUTILS_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QPixmap>
#include <QIcon>
#include <QPainter>
#include <optional>

DFMBASE_BEGIN_NAMESPACE

/**
 * @class IconPainterUtils
 * @brief 图标绘制工具类
 *
 * 为 ItemDelegateHelper、CanvasItemDelegate、CollectionItemDelegate
 * 提供统一的图标绘制和缩略图判断能力，消除三份重复代码。
 *
 * 包含：
 * - PaintIconOpts 公共结构体
 * - visualAlignment / getIconPixmap / paintIcon 绘制工具
 * - isThumbnailIcon 缩略图判断工具
 */
class IconPainterUtils
{
public:
    using ViewMode = Global::ViewMode;

    struct PaintIconOpts
    {
        QRectF rect;
        Qt::Alignment alignment { Qt::AlignCenter };
        QIcon::Mode mode { QIcon::Normal };
        QIcon::State state { QIcon::Off };
        bool isThumb { false };
        QString iconName;   // 用于 QPixmapCache 缓存 key，非缩略图时填入
        ViewMode viewMode { ViewMode::kNoneMode };
    };

    IconPainterUtils() = delete;
    ~IconPainterUtils() = delete;
    Q_DISABLE_COPY(IconPainterUtils)

    /**
     * @brief 文本对齐辅助函数（QStyle::alignedRect 的拷贝）
     */
    static Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment);

    /**
     * @brief 基于 iconName 获取 icon 的 pixmap（带 QPixmapCache 缓存）
     * @details 主题图标专用路径：通过 IconCacheManager 实现进程级共享缓存。
     *          缩略图不走此路径（使用 QIcon 版本）。
     */
    static QPixmap getIconPixmap(const QString &iconName, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

    /**
     * @brief 基于 QIcon 获取 icon 的 pixmap（缩略图/兼容路径）
     */
    static QPixmap getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio,
                                 QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

    /**
     * @brief 绘制指定区域内 icon 的 pixmap
     * @return 成功绘制返回 painted rect；缩略图绘制失败（空 pixmap）返回 std::nullopt
     */
    static std::optional<QRect> paintIcon(QPainter *painter, const QIcon &icon, const PaintIconOpts &opts);

    /**
     * @brief 判断文件是否已生成缩略图
     * @param info 文件信息指针
     * @return true 如果文件有有效缩略图（排除 AppImage / UAB）
     *
     * @note 替换 BaseItemDelegate/CanvasItemDelegate/CollectionItemDelegate
     *       中重复的 isThumnailIconIndex 方法
     */
    static bool isThumbnailIcon(const FileInfoPointer &info);
};

DFMBASE_END_NAMESPACE

#endif   // ICONPAINTERUTILS_H
