// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_systemdcpuutils.cpp
 * @brief Unit tests for SystemdCpuUtils (utils/systemdcpuutils.cpp) — empty
 *        service name early returns (no subprocess spawned).
 */

#include <gtest/gtest.h>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/systemdcpuutils.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(SystemdCpuUtilsTest, SetCpuQuotaEmptyNameReturnsFalse)
{
    QString err;
    EXPECT_FALSE(SystemdCpuUtils::setCpuQuota(QString(), 50, &err));
    EXPECT_FALSE(err.isEmpty());
}

TEST(SystemdCpuUtilsTest, SetCpuQuotaNegativePercentageReturnsFalse)
{
    QString err;
    EXPECT_FALSE(SystemdCpuUtils::setCpuQuota("test.service", -1, &err));
    EXPECT_FALSE(err.isEmpty());
}

TEST(SystemdCpuUtilsTest, ResetCpuQuotaEmptyNameReturnsFalse)
{
    QString err;
    EXPECT_FALSE(SystemdCpuUtils::resetCpuQuota(QString(), &err));
    EXPECT_FALSE(err.isEmpty());
}
