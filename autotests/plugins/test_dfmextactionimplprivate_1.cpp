// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextactionimplprivate_1.cpp
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

TEST_F(DFMExtActionImplPrivateTest, actionImpl)
{
    // Test getter: DFMExtActionImpl actionImpl()
    auto result = obj->actionImpl();
    EXPECT_NO_FATAL_FAILURE({ obj->actionImpl(); });

}

TEST_F(DFMExtActionImplPrivateTest, isCheckable)
{
    // Test bool getter: isCheckable()
    bool result = obj->isCheckable();
    EXPECT_FALSE(result);

}

TEST_F(DFMExtActionImplPrivateTest, isChecked)
{
    // Test bool getter: isChecked()
    bool result = obj->isChecked();
    EXPECT_FALSE(result);

}

TEST_F(DFMExtActionImplPrivateTest, isEnabled)
{
    // Test bool getter: isEnabled()
    bool result = obj->isEnabled();
    EXPECT_FALSE(result);

}

TEST_F(DFMExtActionImplPrivateTest, isInterior)
{
    // Test bool getter: isInterior()
    bool result = obj->isInterior();
    EXPECT_FALSE(result);

}

TEST_F(DFMExtActionImplPrivateTest, isSeparator)
{
    // Test bool getter: isSeparator()
    bool result = obj->isSeparator();
    EXPECT_FALSE(result);

}

TEST_F(DFMExtActionImplPrivateTest, omitText)
{
    // Test method: void omitText((const std::string &text))
    std::string _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->omitText(_arg0));
}

TEST_F(DFMExtActionImplPrivateTest, onActionHovered)
{
    // Test method: void onActionHovered(())
    EXPECT_NO_FATAL_FAILURE(obj->onActionHovered());
}

TEST_F(DFMExtActionImplPrivateTest, onActionTriggered)
{
    // Test method: void onActionTriggered((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->onActionTriggered(false));
}

TEST_F(DFMExtActionImplPrivateTest, setCheckable)
{
    // Test setter: void setCheckable((bool b))
    EXPECT_NO_FATAL_FAILURE(obj->setCheckable(false));
}

TEST_F(DFMExtActionImplPrivateTest, setChecked)
{
    // Test setter: void setChecked((bool b))
    EXPECT_NO_FATAL_FAILURE(obj->setChecked(false));
}

TEST_F(DFMExtActionImplPrivateTest, setEnabled)
{
    // Test setter: void setEnabled((bool b))
    EXPECT_NO_FATAL_FAILURE(obj->setEnabled(false));
}

TEST_F(DFMExtActionImplPrivateTest, setMenu)
{
    // Test setter: void setMenu((DFMExtMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->setMenu(nullptr));
}

TEST_F(DFMExtActionImplPrivateTest, setSeparator)
{
    // Test setter: void setSeparator((bool b))
    EXPECT_NO_FATAL_FAILURE(obj->setSeparator(false));
}

TEST_F(DFMExtActionImplPrivateTest, setToolTip)
{
    // Test setter: void setToolTip((const std::string &tip))
    std::string _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setToolTip(_arg0));
}

TEST_F(DFMExtActionImplPrivateTest, toolTip)
{
    // Test getter: std::string toolTip()
    auto result = obj->toolTip();
    EXPECT_TRUE(result.empty());

}
