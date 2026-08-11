// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collationstrategyprovider.h"
#include "icucollationstrategy.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/dconfig/global_dconf_defines.h>

#include <QLocale>

DFMBASE_BEGIN_NAMESPACE

using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

CollationStrategyProvider *CollationStrategyProvider::instance()
{
    static CollationStrategyProvider ins;
    return &ins;
}

CollationStrategyProvider::CollationStrategyProvider(QObject *parent)
    : QObject(parent)
{
    connect(DConfigManager::instance(), &DConfigManager::valueChanged,
            this, &CollationStrategyProvider::onDConfigChanged);
}

bool CollationStrategyProvider::latinFirstEnabled() const
{
    return DConfigManager::instance()->value(kDefaultCfgPath, kSortLatinFirstZhCn, false).toBool();
}

bool CollationStrategyProvider::isZhCn()
{
    const QLocale loc = QLocale::system();
    return loc.language() == QLocale::Chinese && loc.country() == QLocale::China;
}

std::unique_ptr<CollationStrategy> CollationStrategyProvider::createStrategy() const
{
    // 开关开启且 zh_CN 时清空脚本重排（拉丁在前）；否则保留 CLDR 默认（与原
    // QCollator 行为一致）。两路径均使用 ICU 直出 QByteArray 排序键，避免
    // std::variant 包装带来的排序热路径性能开销。
    const bool clearReorder = latinFirstEnabled() && isZhCn();
    return std::make_unique<IcuCollationStrategy>(clearReorder);
}

const CollationStrategy &CollationStrategyProvider::strategy()
{
    thread_local std::unique_ptr<CollationStrategy> tlsStrategy;
    thread_local uint64_t tlsGeneration = 0;

    const uint64_t gen = m_generation.load(std::memory_order_acquire);
    if (!tlsStrategy || tlsGeneration != gen) {
        tlsStrategy = createStrategy();
        tlsGeneration = gen;
    }
    return *tlsStrategy;
}

void CollationStrategyProvider::onDConfigChanged(const QString &config, const QString &key)
{
    if (config != kDefaultCfgPath || key != kSortLatinFirstZhCn)
        return;

    m_generation.fetch_add(1, std::memory_order_release);
    Q_EMIT strategyChanged();
}

DFMBASE_END_NAMESPACE
