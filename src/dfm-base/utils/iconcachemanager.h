// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONCACHEMANAGER_H
#define ICONCACHEMANAGER_H

#include <dfm-base/dfm_base_global.h>

#include <QPixmap>
#include <QIcon>

DFMBASE_BEGIN_NAMESPACE

/**
 * @class IconCacheManager
 * @brief 基于 QPixmapCache 的图标缓存管理器
 *
 * 参考 Dolphin 的 `KStandardItemListWidget::pixmapForIcon` 设计，
 * 为三个 delegate（ItemDelegateHelper、CanvasItemDelegate、CollectionItemDelegate）
 * 提供统一的图标缓存能力。
 *
 * 核心策略：
 * - 基于 iconName 的 QPixmapCache 缓存（进程级共享）
 * - 缩略图不走此缓存（由 dfm-io / 缩略图服务管理）
 * - 主题变化 / DPR 变化时清空所有缓存（延迟去重执行）
 *
 * 缓存 Key 格式: "dfm:icon:{iconName}:{width}x{height}:dpr{ratio}:{mode}:{state}"
 *
 * @note 线程安全：QPixmapCache 是线程安全的，但 QIcon::pixmap() 应在主线程调用
 */
class IconCacheManager
{
public:
    /**
     * @brief 初始化主题 / DPR 变化监听
     * @note 幂等且线程安全；通常由 getPixmap() 首次调用时懒触发，无需手动调用
     */
    static void initialize();

    /**
     * @brief 获取缓存的图标 pixmap
     * @param iconName          图标名称（如 "video-mp4", "text-plain"）
     * @param size              目标尺寸（逻辑像素）
     * @param devicePixelRatio  设备像素比，参与缓存 key 以区分不同 DPR 屏幕
     * @param mode              图标模式（Normal / Disabled / Active / Selected）
     * @param state             图标状态（On / Off）
     * @return 缓存的 QPixmap（未命中时按设备分辨率从主题加载并写入缓存）
     */
    static QPixmap getPixmap(const QString &iconName,
                             const QSize &size,
                             qreal devicePixelRatio = 1.0,
                             QIcon::Mode mode = QIcon::Normal,
                             QIcon::State state = QIcon::Off);

    /**
     * @brief 清空所有图标缓存（主题 / DPR 变化时调用）
     * @note 带去重保护：多次快速调用只执行一次；
     *       延迟到下一个事件循环执行，避免在绘制周期中清理。
     */
    static void clear();

    /**
     * @brief 构建缓存 Key
     * @return "dfm:icon:{iconName}:{width}x{height}:dpr{ratio}:{mode}:{state}"
     */
    static QString makeCacheKey(const QString &iconName,
                                const QSize &size,
                                qreal devicePixelRatio,
                                QIcon::Mode mode,
                                QIcon::State state);

private:
    IconCacheManager() = delete;
    ~IconCacheManager() = delete;
    Q_DISABLE_COPY(IconCacheManager)
};

DFMBASE_END_NAMESPACE

#endif   // ICONCACHEMANAGER_H
