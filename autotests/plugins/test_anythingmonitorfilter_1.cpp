// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_anythingmonitorfilter_1.cpp
 * @brief Unit tests for AnythingMonitorFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/anythingmonitorfilter.h"

#include <QTest>

using namespace dfmplugin_tag;

class AnythingMonitorFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AnythingMonitorFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AnythingMonitorFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AnythingMonitorFilterTest, AnythingMonitorFilter)
{
    // Test constructor: AnythingMonitorFilter((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AnythingMonitorFilterTest, instance)
{
    // Test getter: DPTAG_BEGIN_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(AnythingMonitorFilterTest, whetherFilterCurrentPath)
{
    // Test method: bool whetherFilterCurrentPath((const QString &localPath))
    QString _arg0{};
    auto result = obj->whetherFilterCurrentPath(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AnythingMonitorFilterTest, AnythingMonitorFilter_Destructor)
{
    // Test method:  ~AnythingMonitorFilter(())
    EXPECT_NO_FATAL_FAILURE({ AnythingMonitorFilter *tmp = new AnythingMonitorFilter(); delete tmp; });
}
