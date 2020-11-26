#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "interfaces/dfmfilepreview.h"

DFM_USE_NAMESPACE
namespace  {

    class MockDFMFilePreview: public DFMFilePreview
    {
    public:
        explicit MockDFMFilePreview(QObject *parent = nullptr): DFMFilePreview(parent)
        {

        }

        bool setFileUrl(const DUrl &url){
            m_url = url;
            return true;
        }
        DUrl fileUrl() const {
            return m_url;
        }

        QWidget *contentWidget() const {
            return 0;
        }
    private:
        DUrl m_url;
    };
    class TestDFMFilePreview : public testing::Test
    {
        void SetUp() override
        {
            std::cout << "start TestDFMFilePreview" << std::endl;
            m_preview = new MockDFMFilePreview();
        }

        void TearDown() override
        {
            std::cout << "end TestDFMFilePreview" << std::endl;
            delete m_preview;
            m_preview = nullptr;
        }
    public:
        DFMFilePreview *m_preview;

    };


TEST_F(TestDFMFilePreview, test_filePreview)
{
    m_preview->initialize(nullptr, nullptr);
    m_preview->statusBarWidget();
    m_preview->statusBarWidgetAlignment();
    m_preview->title() ;
    m_preview->showStatusBarSeparator();
    m_preview->DoneCurrent();
    m_preview->play();
    m_preview->pause();
    m_preview->stop();
    DFMGlobal::clearClipboard();
    DUrl tmpUrl = DUrl::fromLocalFile("/tmp");
    EXPECT_TRUE(m_preview->setFileUrl(tmpUrl));
    m_preview->copyFile();
    QList<QUrl> clipUrls = DFMGlobal::fetchUrlsFromClipboard();
    EXPECT_EQ(clipUrls.first().toString(), tmpUrl.toString());
    EXPECT_EQ(m_preview->contentWidget(), nullptr);
}
}
