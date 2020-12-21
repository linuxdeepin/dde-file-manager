#include <gtest/gtest.h>

#include "textpreviewplugin.h"

DFM_USE_NAMESPACE

class TestTextPreviewPlugin :public testing::Test
{
public:
     void SetUp() override
     {
        m_testPreviewPlugin = new TextPreviewPlugin();
     }

     void TearDown() override
     {
         delete m_testPreviewPlugin;
         m_testPreviewPlugin = nullptr;
     }

public:
     TextPreviewPlugin * m_testPreviewPlugin;
};

TEST_F(TestTextPreviewPlugin, use_create)
{
    EXPECT_TRUE(m_testPreviewPlugin->create("dde-file-manager") != nullptr);
}
