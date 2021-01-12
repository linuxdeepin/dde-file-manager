#include <gtest/gtest.h>

#include "pdfpreviewplugin.h"


class TestPDFPreviewPlugin : public testing::Test {
public:

    void SetUp() override
    {
        m_pdfPreviewPlugin = new PDFPreviewPlugin();
    }

    void TearDown() override
    {
        delete m_pdfPreviewPlugin;
        m_pdfPreviewPlugin = nullptr;
    }

public:
    PDFPreviewPlugin * m_pdfPreviewPlugin;
};

TEST_F(TestPDFPreviewPlugin, use_create)
{
    EXPECT_TRUE(m_pdfPreviewPlugin->create("dde-file-manager") != nullptr);
}
