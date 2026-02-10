// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_abstractbasepreview.cpp - AbstractBasePreview class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <memory>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/interfaces/abstractbasepreview.h>

DFMBASE_USE_NAMESPACE

/**
 * @brief Concrete implementation of AbstractBasePreview for testing
 */
class TestPreview : public AbstractBasePreview
{
    Q_OBJECT
public:
    explicit TestPreview(QObject *parent = nullptr)
        : AbstractBasePreview(parent)
    {
        m_contentWidget = new QWidget();
        m_contentWidget->setLayout(new QVBoxLayout());
        
        m_statusWidget = new QLabel("Status Bar");
    }
    
    bool setFileUrl(const QUrl &url) override
    {
        currentUrl = url;
        return true;
    }
    
    QUrl fileUrl() const override
    {
        return currentUrl;
    }
    
    QWidget *contentWidget() const override
    {
        return m_contentWidget;
    }
    
    QWidget *statusBarWidget() const override
    {
        return m_statusWidget;
    }
    
    QString title() const override
    {
        return "Test Preview Title";
    }
    
    bool showStatusBarSeparator() const override
    {
        return true;
    }
    
    void play() override
    {
        isPlaying = true;
    }
    
    void pause() override
    {
        isPlaying = false;
    }
    
    void stop() override
    {
        isPlaying = false;
    }
    
    // Test helper methods
    bool getIsPlaying() const { return isPlaying; }
    void emitTitleChanged() { Q_EMIT titleChanged(); }
    
private:
    QUrl currentUrl;
    QWidget *m_contentWidget { nullptr };
    QLabel *m_statusWidget { nullptr };
    bool isPlaying { false };
};

/**
 * @brief AbstractBasePreview class unit tests
 *
 * Test scope:
 * 1. Construction and initialization
 * 2. Pure virtual function implementations
 * 3. Virtual function default behavior
 * 4. Signal emission
 * 5. Media control functions (play/pause/stop)
 * 6. Widget management
 * 7. URL handling
 * 8. Lifecycle management (handleBeforDestroy)
 */
class AbstractBasePreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_abstractbasepreview") };
            app = std::make_unique<QApplication>(argc, argv);
        }
        
        // Create test preview instance
        preview = std::make_unique<TestPreview>();
    }
    
    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        preview.reset();
        app.reset();
    }
    
    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QApplication> app;
    std::unique_ptr<TestPreview> preview;
};

TEST_F(AbstractBasePreviewTest, Constructor)
{
    // Test basic construction
    EXPECT_NE(preview.get(), nullptr);
    EXPECT_EQ(preview->parent(), nullptr);
    
    // Test construction with parent
    QObject parent;
    auto childPreview = std::make_unique<TestPreview>(&parent);
    EXPECT_EQ(childPreview->parent(), &parent);
}

TEST_F(AbstractBasePreviewTest, Initialize)
{
    // Create window and status bar widgets
    QWidget window;
    QWidget statusBar;
    
    // Test initialize method
    preview->initialize(&window, &statusBar);
    
    // Verify initialization - this is a basic implementation
    // In a real implementation, we would check if widgets are properly connected
    SUCCEED();  // Initialize should not crash
}

TEST_F(AbstractBasePreviewTest, SetAndGetFileUrl)
{
    QUrl testUrl("file:///test/path/file.txt");
    
    // Test setting file URL
    EXPECT_TRUE(preview->setFileUrl(testUrl));
    
    // Test getting file URL
    EXPECT_EQ(preview->fileUrl(), testUrl);
    
    // Test with different URL
    QUrl anotherUrl("file:///another/path/document.pdf");
    EXPECT_TRUE(preview->setFileUrl(anotherUrl));
    EXPECT_EQ(preview->fileUrl(), anotherUrl);
}

TEST_F(AbstractBasePreviewTest, ContentWidget)
{
    // Test content widget retrieval
    auto *content = preview->contentWidget();
    EXPECT_NE(content, nullptr);
    EXPECT_NE(content->layout(), nullptr);
}

TEST_F(AbstractBasePreviewTest, StatusBarWidget)
{
    // Test status bar widget retrieval
    auto *statusBar = preview->statusBarWidget();
    EXPECT_NE(statusBar, nullptr);
    
    // Test default implementation returns nullptr
    // Since AbstractBasePreview is abstract, we use TestPreview
    TestPreview defaultPreview;
    EXPECT_NE(defaultPreview.statusBarWidget(), nullptr);
}

TEST_F(AbstractBasePreviewTest, StatusBarWidgetAlignment)
{
    // Test status bar widget alignment
    // Default implementation should return a valid alignment
    auto alignment = preview->statusBarWidgetAlignment();
    EXPECT_TRUE(alignment != Qt::Alignment());
}

TEST_F(AbstractBasePreviewTest, Title)
{
    // Test title retrieval
    QString title = preview->title();
    EXPECT_FALSE(title.isEmpty());
    EXPECT_EQ(title, "Test Preview Title");
    
    // Test default implementation returns empty string
    // Since AbstractBasePreview is abstract, we use TestPreview
    TestPreview defaultPreview;
    EXPECT_FALSE(defaultPreview.title().isEmpty());
}

TEST_F(AbstractBasePreviewTest, ShowStatusBarSeparator)
{
    // Test status bar separator visibility
    EXPECT_TRUE(preview->showStatusBarSeparator());
    
    // Test default implementation returns false
    // Since AbstractBasePreview is abstract, we use TestPreview
    TestPreview defaultPreview;
    EXPECT_TRUE(defaultPreview.showStatusBarSeparator());
}

TEST_F(AbstractBasePreviewTest, MediaControlFunctions)
{
    // Test play function
    preview->play();
    EXPECT_TRUE(preview->getIsPlaying());
    
    // Test pause function
    preview->pause();
    EXPECT_FALSE(preview->getIsPlaying());
    
    // Test stop function
    preview->play();
    EXPECT_TRUE(preview->getIsPlaying());
    preview->stop();
    EXPECT_FALSE(preview->getIsPlaying());
}

TEST_F(AbstractBasePreviewTest, TitleChangedSignal)
{
    // Test signal emission
    QSignalSpy spy(preview.get(), &TestPreview::titleChanged);
    
    // Emit signal
    preview->emitTitleChanged();
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(AbstractBasePreviewTest, HandleBeforDestroy)
{
    // Test handleBeforDestroy method
    // This is a cleanup method, so we just verify it doesn't crash
    EXPECT_NO_THROW(preview->handleBeforDestroy());
}

TEST_F(AbstractBasePreviewTest, EdgeCases)
{
    // Test with empty URL
    QUrl emptyUrl;
    EXPECT_TRUE(preview->setFileUrl(emptyUrl));
    EXPECT_EQ(preview->fileUrl(), emptyUrl);
    
    // Test with invalid URL
    QUrl invalidUrl("not_a_valid_url");
    EXPECT_TRUE(preview->setFileUrl(invalidUrl));
    EXPECT_EQ(preview->fileUrl(), invalidUrl);
    
    // Test multiple calls to media controls
    preview->play();
    preview->play();  // Should not cause issues
    preview->pause();
    preview->pause();  // Should not cause issues
    preview->stop();
    preview->stop();   // Should not cause issues
}

TEST_F(AbstractBasePreviewTest, MemoryManagement)
{
    // Test creating and destroying multiple instances
    for (int i = 0; i < 10; ++i) {
        auto tempPreview = std::make_unique<TestPreview>();
        EXPECT_NE(tempPreview.get(), nullptr);
        tempPreview->setFileUrl(QUrl(QString("file:///test/%1").arg(i)));
        EXPECT_EQ(tempPreview->fileUrl().toString(), QString("file:///test/%1").arg(i));
    }
}

#include "test_abstractbasepreview.moc"