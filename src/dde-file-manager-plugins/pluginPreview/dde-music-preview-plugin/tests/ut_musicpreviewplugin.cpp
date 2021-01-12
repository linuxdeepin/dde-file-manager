#include <gtest/gtest.h>

#include "musicpreviewplugin.h"


class TestMusicPreviewPlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_musicPreviewPlugin = new MusicPreviewPlugin();
    }

    void TearDown() override
    {
        delete m_musicPreviewPlugin;
        m_musicPreviewPlugin = nullptr;
    }

public:
    MusicPreviewPlugin * m_musicPreviewPlugin;
};

TEST_F(TestMusicPreviewPlugin, use_create)
{
    EXPECT_TRUE(m_musicPreviewPlugin->create("dde-file-manager") != nullptr);
}
