// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconcachemanager.h"

#include <dtkwidget_global.h>
#include <DGuiApplicationHelper>

#include <QApplication>
#include <QScreen>
#include <QPixmapCache>
#include <QTimer>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DGUI_USE_NAMESPACE

/*!
 * \brief clearTimer 单例清理定时器
 *
 * 使用 C++11 魔法静态（线程安全）保证定时器及其 timeout 连接只创建一次，
 * 避免 clear() 中以普通 static bool 守卫连接所带来的竞态（MUL-xxx #16）。
 * 多次 start() 会重置已存在的单次定时器，从而合并连续调用。
 */
static QTimer *clearTimer()
{
    static QTimer *timer = []() {
        auto *t = new QTimer(qApp);
        t->setSingleShot(true);
        t->setInterval(0);   // 下一个事件循环执行
        QObject::connect(t, &QTimer::timeout, []() {
            QPixmapCache::clear();
            qCDebug(logDFMBase) << "[IconCache] Pixmap cache cleared due to theme/DPR change";
        });
        return t;
    }();
    return timer;
}

// 将单个屏幕的逻辑 DPI 变化信号连接到 clear()，DPR 变化时使缓存失效
static void connectScreenDpiChanged(QScreen *screen)
{
    if (!screen)
        return;
    QObject::connect(screen, &QScreen::logicalDotsPerInchChanged,
                     &IconCacheManager::clear);
}

void IconCacheManager::initialize()
{
    // 幂等且线程安全：魔法静态保证连接逻辑仅执行一次
    static const bool s_done = []() {
        // 监听 DTK 主题类型变化（深色/浅色切换）
        QObject::connect(DGuiApplicationHelper::instance(),
                         &DGuiApplicationHelper::themeTypeChanged,
                         &IconCacheManager::clear);

        // 监听 Qt 调色板变化（兼容非 DTK 场景，如只改强调色不改主题类型）
        QObject::connect(qApp, &QApplication::paletteChanged,
                         &IconCacheManager::clear);

        // DPR 相关：主屏切换 / 屏幕增删 / 屏幕逻辑 DPI 变化时清空缓存
        QObject::connect(qApp, &QGuiApplication::primaryScreenChanged,
                         &IconCacheManager::clear);
        QObject::connect(qApp, &QGuiApplication::screenAdded,
                         [](QScreen *screen) { connectScreenDpiChanged(screen); });
        const auto screens = QGuiApplication::screens();
        for (QScreen *s : screens)
            connectScreenDpiChanged(s);

        qCDebug(logDFMBase) << "[IconCache] IconCacheManager initialized";
        return true;
    }();
    Q_UNUSED(s_done)
}

QPixmap IconCacheManager::getPixmap(const QString &iconName,
                                    const QSize &size,
                                    qreal devicePixelRatio,
                                    QIcon::Mode mode,
                                    QIcon::State state)
{
    // C++11 线程安全的懒初始化：首次调用时自动建立主题/DPR 监听连接
    static const bool s_initialized = []() {
        initialize();
        return true;
    }();
    Q_UNUSED(s_initialized)

    if (iconName.isEmpty() || !size.isValid() || size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    const QString key = makeCacheKey(iconName, size, devicePixelRatio, mode, state);

    // 尝试从 QPixmapCache 命中
    QPixmap px;
    if (QPixmapCache::find(key, &px))
        return px;

    // 未命中：按设备分辨率从主题加载（与 v20 getIconPixmap 行为一致）
    QIcon icon = QIcon::fromTheme(iconName);
    if (icon.isNull())
        return QPixmap();

    px = icon.pixmap(size * devicePixelRatio, mode, state);
    if (!px.isNull()) {
        px.setDevicePixelRatio(devicePixelRatio);
        QPixmapCache::insert(key, px);
    }

    return px;
}

void IconCacheManager::clear()
{
    // 去重：单次定时器 start() 合并多次连续调用，延迟到下一个事件循环执行
    clearTimer()->start();
}

QString IconCacheManager::makeCacheKey(const QString &iconName,
                                       const QSize &size,
                                       qreal devicePixelRatio,
                                       QIcon::Mode mode,
                                       QIcon::State state)
{
    // Key 格式: "dfm:icon:{name}:{width}x{height}:dpr{ratio}:{mode}:{state}"
    // DPR 纳入 key，区分不同 DPR 屏幕下的渲染结果；DPR 变化由 clear() 覆盖
    return QString("dfm:icon:%1:%2x%3:dpr%4:%5:%6")
        .arg(iconName)
        .arg(size.width())
        .arg(size.height())
        .arg(devicePixelRatio)
        .arg(static_cast<int>(mode))
        .arg(static_cast<int>(state));
}
