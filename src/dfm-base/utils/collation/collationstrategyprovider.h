// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLATIONSTRATEGYPROVIDER_H
#define COLLATIONSTRATEGYPROVIDER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <atomic>
#include <memory>

DFMBASE_BEGIN_NAMESPACE

class CollationStrategy;

/**
 * @class CollationStrategyProvider
 * @brief 排序策略选择与运行时切换的单例提供者。
 *
 * 职责（SRP）：
 * - 读取 dconfig 开关（dfm.sort.latinfirst.zhCn）+ 判定系统 locale，
 *   决定创建的 IcuCollationStrategy 是否清空脚本重排（clearReorder）。
 * - 监听 DConfigManager::valueChanged，开关变化时使代际失效并发出
 *   strategyChanged 信号，供视图连接后触发重排。
 *
 * 线程安全：strategy() 返回 thread_local 策略实例，配合代际计数器实现
 * 无锁的线程安全策略访问。代际变更（dconfig 改动）后，各线程下次调用
 * strategy() 时自动按新策略重建线程局部实例。
 */
class CollationStrategyProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CollationStrategyProvider)

public:
    static CollationStrategyProvider *instance();

    /// 获取当前线程的策略实例（thread_local，代际过期时自动重建）
    const CollationStrategy &strategy();

    /// dconfig 开关是否开启（dfm.sort.latinfirst.zhCn）
    bool latinFirstEnabled() const;

Q_SIGNALS:
    /// 策略可能已变更（dconfig 开关变化），视图应触发重排
    void strategyChanged();

private:
    explicit CollationStrategyProvider(QObject *parent = nullptr);
    ~CollationStrategyProvider() override = default;

    /// 按当前 dconfig + locale 创建策略实例
    std::unique_ptr<CollationStrategy> createStrategy() const;

    /// dconfig 变更回调
    void onDConfigChanged(const QString &config, const QString &key);

    /// 判定系统 locale 是否为简体中文
    static bool isZhCn();

private:
    /// 代际计数器：每次策略可能变更时自增，触发线程局部策略重建
    std::atomic<uint64_t> m_generation { 0 };
};

DFMBASE_END_NAMESPACE

#endif   // COLLATIONSTRATEGYPROVIDER_H
