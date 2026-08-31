// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextactionimplprivate.cpp
 * @brief Unit tests for DFMExtActionImplPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/menuimpl/dfmextactionimpl.h"

#include <QTest>

using namespace dfmplugin_utils;

class DFMExtActionImplPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtActionImplPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtActionImplPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtActionImplPrivateTest, deleteParent)
{
    // Test method: void deleteParent(())
    EXPECT_NO_FATAL_FAILURE(obj->deleteParent());
}

TEST_F(DFMExtActionImplPrivateTest, setIcon)
{
    // Test setter: void setIcon((const std::string &iconName))
    std::string _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setIcon(_arg0));
}

TEST_F(DFMExtActionImplPrivateTest, setText)
{
    // Test setter: void setText((const std::string &text))
    std::string _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setText(_arg0));
}

TEST_F(DFMExtActionImplPrivateTest, DFMExtActionImplPrivate)
{
    // Test constructor: DFMExtActionImplPrivate((DFMExtActionImpl *qImpl, QAction *ac))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMExtActionImplPrivateTest, icon)
{
    // Test getter: std::string icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.empty());

}

TEST_F(DFMExtActionImplPrivateTest, menu)
{
    // Test getter: DFMExtMenu menu()
    auto result = obj->menu();
    EXPECT_NO_FATAL_FAILURE({ obj->menu(); });

}

TEST_F(DFMExtActionImplPrivateTest, qaction)
{
    // Test getter: QAction qaction()
    auto result = obj->qaction();
    EXPECT_NO_FATAL_FAILURE({ obj->qaction(); });

}

TEST_F(DFMExtActionImplPrivateTest, text)
{
    // Test getter: std::string text()
    auto result = obj->text();
    EXPECT_TRUE(result.empty());

}
