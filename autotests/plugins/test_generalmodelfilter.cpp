// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_generalmodelfilter.cpp
 * @brief Unit tests for GeneralModelFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/generalmodelfilter.h"

#include <QTest>

using namespace ddplugin_organizer;

class GeneralModelFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GeneralModelFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GeneralModelFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GeneralModelFilterTest, acceptReset)
{
    // Test method: QList<QUrl> acceptReset((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->acceptReset(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(GeneralModelFilterTest, removeFilter)
{
    // Test method: void removeFilter((ModelDataHandler *filter))
    EXPECT_NO_FATAL_FAILURE(obj->removeFilter(nullptr));
}
