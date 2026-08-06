// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deviceutils_r16.cpp
 * @brief Additional DeviceUtils tests: isSiblingOfRoot(QVariantMap) overload
 *        (covers the QMap overload that delegates to the QHash version).
 */

#include <gtest/gtest.h>
#include <QVariantMap>
#include <QHash>
#include <QString>

#include <dfm-base/base/device/deviceutils.h>

using namespace dfmbase;

TEST(DeviceUtilsR16Test, IsSiblingOfRootQMapOverload)
{
    QVariantMap infos;
    EXPECT_NO_FATAL_FAILURE({ (void)DeviceUtils::isSiblingOfRoot(infos); });
}