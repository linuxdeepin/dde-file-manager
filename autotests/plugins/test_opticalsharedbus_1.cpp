// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalsharedbus_1.cpp
 * @brief Unit tests for OpticalShareDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "opticalsharedbus.h"

#include <QTest>

using namespace opticalshare;

class OpticalShareDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalShareDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalShareDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalShareDBusTest, GetBurnAttribute)
{
    // Test method: QVariantMap GetBurnAttribute((const QString &tag))
    QString _arg0{};
    auto result = obj->GetBurnAttribute(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalShareDBusTest, GetBurnState)
{
    // Test method: QVariantMap GetBurnState((const QString &dev))
    QString _arg0{};
    auto result = obj->GetBurnState(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalShareDBusTest, GetBurnStates)
{
    // Test getter: QVariantMap GetBurnStates()
    auto result = obj->GetBurnStates();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalShareDBusTest, OpticalShareDBus)
{
    // Test constructor: OpticalShareDBus((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpticalShareDBusTest, SetBurnAttribute)
{
    // Test method: bool SetBurnAttribute((const QString &tag, const QVariantMap &attribute))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->SetBurnAttribute(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OpticalShareDBusTest, SetBurnState)
{
    // Test method: bool SetBurnState((const QString &dev, const QVariantMap &state))
    QString _arg0{};
    QVariantMap _arg1{};
    auto result = obj->SetBurnState(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OpticalShareDBusTest, normalizeBurnAttribute)
{
    // Test method: QVariantMap normalizeBurnAttribute((const QVariantMap &attribute))
    QVariantMap _arg0{};
    auto result = obj->normalizeBurnAttribute(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalShareDBusTest, normalizeBurnState)
{
    // Test method: QVariantMap normalizeBurnState((const QVariantMap &state))
    QVariantMap _arg0{};
    auto result = obj->normalizeBurnState(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalShareDBusTest, OpticalShareDBus_Destructor)
{
    // Test method:  ~OpticalShareDBus(())
    EXPECT_NO_FATAL_FAILURE({ OpticalShareDBus *tmp = new OpticalShareDBus(); delete tmp; });
}
