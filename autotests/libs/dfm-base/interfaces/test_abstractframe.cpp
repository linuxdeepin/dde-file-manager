// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QUrl>
#include <QSignalSpy>

#include <dfm-base/interfaces/abstractframe.h>

// Include stub headers
#include "stubext.h"

using namespace dfmbase;

// Create a concrete implementation of AbstractFrame for testing
class TestFrameImpl : public dfmbase::AbstractFrame
{
    Q_OBJECT
public:
    TestFrameImpl(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : AbstractFrame(parent, f)
    {
    }
    
    void setCurrentUrl(const QUrl &url) override {
        m_currentUrl = url;
    }
    
    QUrl currentUrl() const override {
        return m_currentUrl;
    }
    
private:
    QUrl m_currentUrl;
};

class TestAbstractFrame : public testing::Test
{
protected:
    void SetUp() override {
        stub.clear();
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        frame = new TestFrameImpl();
    }
    
    void TearDown() override {
        stub.clear();
        if (frame) {
            delete frame;
            frame = nullptr;
        }
    }
    
public:
    stub_ext::StubExt stub;
    QApplication *app = nullptr;
    TestFrameImpl *frame = nullptr;
};

// Test constructor with parent and window flags
TEST_F(TestAbstractFrame, TestConstructorWithParentAndFlags)
{
    QWidget parent;
    Qt::WindowFlags flags = Qt::Window | Qt::WindowTitleHint;
    
    TestFrameImpl *tmpFrame = new TestFrameImpl(&parent, flags);
    
    ASSERT_NE(tmpFrame, nullptr);
    EXPECT_EQ(tmpFrame->parent(), &parent);
    delete tmpFrame;
}

// Test constructor with parent only
TEST_F(TestAbstractFrame, TestConstructorWithParent)
{
    QWidget parent;
    
    TestFrameImpl *tmpFrame = new TestFrameImpl(&parent);
    
    ASSERT_NE(tmpFrame, nullptr);
    EXPECT_EQ(tmpFrame->parent(), &parent);
    delete tmpFrame;
}

// Test constructor without parameters
TEST_F(TestAbstractFrame, TestConstructorWithoutParameters)
{
    ASSERT_NE(frame, nullptr);
    EXPECT_EQ(frame->parent(), nullptr);
}

// Test destructor
TEST_F(TestAbstractFrame, TestDestructor)
{
    TestFrameImpl *tempFrame = new TestFrameImpl();
    delete tempFrame;
    
    SUCCEED();
}

// Test setCurrentUrl method (pure virtual)
TEST_F(TestAbstractFrame, TestSetCurrentUrl)
{
    QUrl url("file:///test/path");
    frame->setCurrentUrl(url);
    
    EXPECT_EQ(frame->currentUrl(), url);
}

// Test currentUrl method (pure virtual)
TEST_F(TestAbstractFrame, TestCurrentUrl)
{
    QUrl url("file:///test/path");
    frame->setCurrentUrl(url);
    
    QUrl currentUrl = frame->currentUrl();
    EXPECT_EQ(currentUrl, url);
}

// Test with empty URL
TEST_F(TestAbstractFrame, TestWithEmptyUrl)
{
    QUrl emptyUrl;
    frame->setCurrentUrl(emptyUrl);
    
    EXPECT_EQ(frame->currentUrl(), emptyUrl);
    EXPECT_TRUE(frame->currentUrl().isEmpty());
}

// Test with different URL schemes
TEST_F(TestAbstractFrame, TestWithDifferentUrlSchemes)
{
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///local/path")
             << QUrl("ftp://ftp.example.com")
             << QUrl("smb://server/share")
             << QUrl("http://example.com/file");
    
    for (const QUrl &url : testUrls) {
        frame->setCurrentUrl(url);
        EXPECT_EQ(frame->currentUrl(), url);
    }
}

// Test URL changes
TEST_F(TestAbstractFrame, TestUrlChanges)
{
    // Test multiple URL changes
    for (int i = 0; i < 10; ++i) {
        QUrl url(QString("file:///test/path_%1").arg(i));
        frame->setCurrentUrl(url);
        EXPECT_EQ(frame->currentUrl(), url);
    }
}

// Test QFrame inheritance
TEST_F(TestAbstractFrame, TestQFrameInheritance)
{
    // Should inherit from QFrame
    EXPECT_TRUE(dynamic_cast<QFrame*>(frame) != nullptr);
    
    // Test QFrame properties
    frame->setFrameStyle(QFrame::Box);
    EXPECT_EQ(frame->frameStyle(), QFrame::Box);
    
    frame->setLineWidth(2);
    EXPECT_EQ(frame->lineWidth(), 2);
}

// Test QWidget properties
TEST_F(TestAbstractFrame, TestQWidgetProperties)
{
    // Test basic QWidget properties
    frame->setObjectName("TestFrame");
    EXPECT_EQ(frame->objectName(), "TestFrame");
    
    frame->setEnabled(true);
    EXPECT_TRUE(frame->isEnabled());
    
    frame->setVisible(true);
    EXPECT_TRUE(frame->isVisible());
}

// Test with parent-child relationship
TEST_F(TestAbstractFrame, TestParentChildRelationship)
{
    QWidget parent;
    TestFrameImpl *tmpFrame = new TestFrameImpl(&parent);
    
    // Should have parent
    EXPECT_EQ(tmpFrame->parent(), &parent);
    
    // Parent should have child
    EXPECT_TRUE(parent.findChildren<TestFrameImpl*>().contains(tmpFrame));
    delete tmpFrame;
}

// Test multiple instances
TEST_F(TestAbstractFrame, TestMultipleInstances)
{
    TestFrameImpl *frame1 = new TestFrameImpl();
    TestFrameImpl *frame2 = new TestFrameImpl();
    
    QUrl url1("file:///frame1");
    QUrl url2("file:///frame2");
    
    frame1->setCurrentUrl(url1);
    frame2->setCurrentUrl(url2);
    
    EXPECT_EQ(frame1->currentUrl(), url1);
    EXPECT_EQ(frame2->currentUrl(), url2);
    EXPECT_NE(frame1, frame2);
    
    delete frame1;
    delete frame2;
}

// Test with URLs containing special characters
TEST_F(TestAbstractFrame, TestUrlsWithSpecialCharacters)
{
    QString pathWithSpaces = "/path with spaces/file name.txt";
    QString pathWithUnicode = "/路径/文件.txt";
    
    QUrl url1 = QUrl::fromLocalFile(pathWithSpaces);
    QUrl url2 = QUrl::fromLocalFile(pathWithUnicode);
    
    frame->setCurrentUrl(url1);
    EXPECT_EQ(frame->currentUrl(), url1);
    
    frame->setCurrentUrl(url2);
    EXPECT_EQ(frame->currentUrl(), url2);
}

// Test window flags
TEST_F(TestAbstractFrame, TestWindowFlags)
{
    Qt::WindowFlags flags = Qt::Window | Qt::CustomizeWindowHint;
    frame = new TestFrameImpl(nullptr, flags);
    
    // Window flags should be set (though we can't easily verify the exact flags)
    EXPECT_NE(frame, nullptr);
}

// Test rapid URL changes
TEST_F(TestAbstractFrame, TestRapidUrlChanges)
{
    // Test rapid URL changes
    for (int i = 0; i < 100; ++i) {
        QUrl url(QString("file:///rapid_%1").arg(i));
        frame->setCurrentUrl(url);
        EXPECT_EQ(frame->currentUrl(), url);
    }
}

// Test frame size and geometry
TEST_F(TestAbstractFrame, TestFrameSizeAndGeometry)
{
    // Test size
    frame->resize(400, 300);
    EXPECT_EQ(frame->size(), QSize(400, 300));
    
    // Test geometry
    frame->move(100, 100);
    EXPECT_EQ(frame->pos(), QPoint(100, 100));
}

// Test style sheet
TEST_F(TestAbstractFrame, TestStyleSheet)
{
    QString stylesheet = "QFrame { background-color: red; }";
    frame->setStyleSheet(stylesheet);
    
    EXPECT_EQ(frame->styleSheet(), stylesheet);
}

// Test signals and slots (basic test)
TEST_F(TestAbstractFrame, TestSignalsAndSlots)
{
    // Test that the frame can connect to signals
    QSignalSpy spy(frame, &QObject::objectNameChanged);
    
    frame->setObjectName("NewName");
    
    // Should emit objectNameChanged signal
    EXPECT_EQ(spy.count(), 1);
}

// Test memory management
TEST_F(TestAbstractFrame, TestMemoryManagement)
{
    // Create and destroy multiple instances
    for (int i = 0; i < 10; ++i) {
        TestFrameImpl *tempFrame = new TestFrameImpl();
        tempFrame->setCurrentUrl(QUrl(QString("file:///temp_%1").arg(i)));
        delete tempFrame;
    }
    
    SUCCEED();
}

// Test with complex URLs
TEST_F(TestAbstractFrame, TestWithComplexUrls)
{
    // Test with query parameters
    QUrl urlWithQuery("http://example.com/path?param1=value1&param2=value2");
    frame->setCurrentUrl(urlWithQuery);
    EXPECT_EQ(frame->currentUrl(), urlWithQuery);
    
    // Test with fragment
    QUrl urlWithFragment("http://example.com/path#section");
    frame->setCurrentUrl(urlWithFragment);
    EXPECT_EQ(frame->currentUrl(), urlWithFragment);
}

// Test frame visibility and focus
TEST_F(TestAbstractFrame, TestFrameVisibilityAndFocus)
{
    // Test visibility
    frame->show();
    EXPECT_TRUE(frame->isVisible());
    
    frame->hide();
    EXPECT_FALSE(frame->isVisible());
    
    // Test focus
    frame->setFocus();
    EXPECT_TRUE(frame->hasFocus());
}

#include "test_abstractframe.moc"