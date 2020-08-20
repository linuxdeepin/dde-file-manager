#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "views/dfilemanagerwindow.h"

DFM_USE_NAMESPACE
DCORE_USE_NAMESPACE

namespace  {
    class TestFileManagerWindow : public testing::Test
    {
    public:
        DFileManagerWindow *m_fileManagerWindow;
        virtual void SetUp() override
        {
            m_fileManagerWindow = new DFileManagerWindow(DUrl(COMPUTER_ROOT));
            std::cout << "start DFileManagerWindow" << std::endl;
        }

        virtual void TearDown() override
        {
            delete m_fileManagerWindow;
            m_fileManagerWindow = nullptr;
            std::cout << "end DFileManagerWindow" << std::endl;
        }
    };
}

TEST_F(TestFileManagerWindow, can_RequestRedirectUrl)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestRedirectUrl(DUrl(), DUrl()));
}

TEST_F(TestFileManagerWindow, can_RequestCloseTabByUrl)
{
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(DUrl(TRASH_ROOT)));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(DUrl(COMPUTER_ROOT)));
    m_fileManagerWindow->openNewTab(DUrl(COMPUTER_ROOT));
    m_fileManagerWindow->openNewTab(DUrl(COMPUTER_ROOT));
    EXPECT_NO_FATAL_FAILURE(m_fileManagerWindow->onRequestCloseTabByUrl(m_fileManagerWindow->currentUrl()));
}
