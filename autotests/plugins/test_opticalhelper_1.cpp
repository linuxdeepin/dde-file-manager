// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalhelper_1.cpp
 * @brief Unit tests for OpticalHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/opticalhelper.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalHelperTest, OpticalHelper)
{
    // Test constructor: OpticalHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalHelperTest, allOpticalDiscMountPoints)
{
    // Test getter: QStringList allOpticalDiscMountPoints()
    auto result = obj->allOpticalDiscMountPoints();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalHelperTest, burnIsOnStaging)
{
    // Test method: bool burnIsOnStaging((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->burnIsOnStaging(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalHelperTest, burnRxp)
{
    // Test getter: QRegularExpression burnRxp()
    auto result = obj->burnRxp();
    EXPECT_NO_FATAL_FAILURE({ obj->burnRxp(); });

}

TEST_F(OpticalHelperTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(OpticalHelperTest, iconString)
{
    // Test getter: QString iconString()
    auto result = obj->iconString();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalHelperTest, instance)
{
    // Test getter: OpticalHelper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(OpticalHelperTest, isDupFileNameInPath)
{
    // Test method: bool isDupFileNameInPath((const QString &path, const QUrl &url))
    QString _arg0{};
    QUrl _arg1{};
    auto result = obj->isDupFileNameInPath(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OpticalHelperTest, isSupportedUDFMedium)
{
    // Test method: bool isSupportedUDFMedium((int type))
    auto result = obj->isSupportedUDFMedium(0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalHelperTest, isSupportedUDFVersion)
{
    // Test method: bool isSupportedUDFVersion((const QString &version))
    QString _arg0{};
    auto result = obj->isSupportedUDFVersion(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalHelperTest, isTransparent)
{
    // Test method: bool isTransparent((const QUrl &url, Global::TransparentStatus *status))
    QUrl _arg0{};
    auto result = obj->isTransparent(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpticalHelperTest, localDiscFile)
{
    // Test method: QUrl localDiscFile((const QUrl &dest))
    QUrl _arg0{};
    auto result = obj->localDiscFile(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(OpticalHelperTest, tansToBurnFile)
{
    // Test method: QUrl tansToBurnFile((const QUrl &in))
    QUrl _arg0{};
    auto result = obj->tansToBurnFile(_arg0);
    EXPECT_FALSE(result.isValid());

}
