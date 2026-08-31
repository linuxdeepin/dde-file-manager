// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_generalmodelfilter_1.cpp
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

TEST_F(GeneralModelFilterTest, GeneralModelFilter)
{
    // Test constructor: GeneralModelFilter(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(GeneralModelFilterTest, acceptInsert)
{
    // Test method: bool acceptInsert((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->acceptInsert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(GeneralModelFilterTest, acceptRename)
{
    // Test method: bool acceptRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->acceptRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(GeneralModelFilterTest, acceptUpdate)
{
    // Test method: bool acceptUpdate((const QUrl &url, const QVector<int> &roles))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->acceptUpdate(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(GeneralModelFilterTest, installFilter)
{
    // Test method: bool installFilter((ModelDataHandler *filter))
    auto result = obj->installFilter(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(GeneralModelFilterTest, GeneralModelFilter_Destructor)
{
    // Test method:  ~GeneralModelFilter(())
    EXPECT_NO_FATAL_FAILURE({ GeneralModelFilter *tmp = new GeneralModelFilter(); delete tmp; });
}
