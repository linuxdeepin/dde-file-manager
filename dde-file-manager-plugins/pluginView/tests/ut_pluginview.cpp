/*

 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.

 *

 * Author:     lixiang

 *

 * Maintainer: lixianga@uniontech.com

 *

 * brief:

 *

 * date:    2020-08-27

 */


#include <gtest/gtest.h>

#include "viewplugin.h"
#include "durl.h"

#include "stub.h"

class TestViewPlugin :public testing::Test
{
public:
     void SetUp() override
     {
        m_viewPlugin = new ViewPlugin();
     }

     void TearDown() override
     {
        delete m_viewPlugin;
        m_viewPlugin = nullptr;
     }

public:
    ViewPlugin * m_viewPlugin;
};


TEST_F(TestViewPlugin, get_bookMarkText)
{
    EXPECT_FALSE(m_viewPlugin->bookMarkText().isEmpty());
}

TEST_F(TestViewPlugin, get_bookMarkNormalIcon)
{
    EXPECT_FALSE(m_viewPlugin->bookMarkNormalIcon().isNull());
}

TEST_F(TestViewPlugin, get_bookMarkHoverIcon)
{
    EXPECT_FALSE(m_viewPlugin->bookMarkHoverIcon().isNull());
}

TEST_F(TestViewPlugin, get_bookMarkPressedIcon)
{
    EXPECT_FALSE(m_viewPlugin->bookMarkPressedIcon().isNull());
}

TEST_F(TestViewPlugin, get_bookMarkCheckedIcon)
{
    EXPECT_FALSE(m_viewPlugin->bookMarkCheckedIcon().isNull());
}

TEST_F(TestViewPlugin, get_crumbText)
{
    EXPECT_FALSE(m_viewPlugin->crumbText().isEmpty());
}

TEST_F(TestViewPlugin, get_crumbNormalIcon)
{
    EXPECT_FALSE(m_viewPlugin->crumbNormalIcon().isNull());
}

TEST_F(TestViewPlugin, get_crumbHoverIcon)
{
    EXPECT_FALSE(m_viewPlugin->crumbHoverIcon().isNull());
}

TEST_F(TestViewPlugin, get_crumbPressedIcon)
{
    EXPECT_FALSE(m_viewPlugin->crumbPressedIcon().isNull());
}

TEST_F(TestViewPlugin, get_crumbCheckedIcon)
{
    EXPECT_FALSE(m_viewPlugin->crumbCheckedIcon().isNull());
}

TEST_F(TestViewPlugin, get_isAddSeparator)
{
    EXPECT_TRUE(m_viewPlugin->isAddSeparator());
}

TEST_F(TestViewPlugin, get_scheme)
{
    EXPECT_FALSE(m_viewPlugin->scheme().isEmpty());
}

TEST_F(TestViewPlugin, get_createView)
{
    EXPECT_TRUE(m_viewPlugin->createView() != nullptr);
}
