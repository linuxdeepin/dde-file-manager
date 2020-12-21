#include <gtest/gtest.h>

#include "imagepreviewplugin.h"


class TestImagePreviewPlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_imagePreviewPlugin = new ImagePreviewPlugin();
    }

    void TearDown() override
    {
        delete m_imagePreviewPlugin;
        m_imagePreviewPlugin = nullptr;
    }

public:
    ImagePreviewPlugin * m_imagePreviewPlugin;
};

TEST_F(TestImagePreviewPlugin, use_create)
{
    EXPECT_TRUE(m_imagePreviewPlugin->create("dde-file-manager") != nullptr);
}
