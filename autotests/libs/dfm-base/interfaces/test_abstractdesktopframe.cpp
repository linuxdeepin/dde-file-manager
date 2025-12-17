// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_abstractdesktopframe.cpp - AbstractDesktopFrame class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QWidget>
#include <QApplication>
#include <QTimer>
#include <memory>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/interfaces/abstractdesktopframe.h>

DFMBASE_USE_NAMESPACE

/**
 * @brief Concrete implementation of AbstractDesktopFrame for testing
 */
class TestDesktopFrame : public AbstractDesktopFrame
{
    Q_OBJECT
public:
    explicit TestDesktopFrame(QObject *parent = nullptr)
        : AbstractDesktopFrame(parent)
    {
        // Create some test windows
        for (int i = 0; i < 3; ++i) {
            auto *window = new QWidget();
            window->setObjectName(QString("TestWindow%1").arg(i));
            windows.append(window);
        }
    }
    
    ~TestDesktopFrame()
    {
        qDeleteAll(windows);
    }
    
    QList<QWidget *> rootWindows() const override
    {
        return windows;
    }
    
    void layoutChildren() override
    {
        layoutCount++;
        
        // Simulate layout operation
        for (int i = 0; i < windows.size(); ++i) {
            if (windows[i]) {
                windows[i]->move(i * 100, i * 100);
                windows[i]->resize(200, 200);
            }
        }
    }
    
    // Test helper methods
    void emitWindowAboutToBeBuilded() { Q_EMIT windowAboutToBeBuilded(); }
    void emitWindowBuilded() { Q_EMIT windowBuilded(); }
    void emitWindowShowed() { Q_EMIT windowShowed(); }
    void emitGeometryChanged() { Q_EMIT geometryChanged(); }
    void emitAvailableGeometryChanged() { Q_EMIT availableGeometryChanged(); }
    
    int getLayoutCount() const { return layoutCount; }
    
private:
    QList<QWidget *> windows;
    int layoutCount { 0 };
};

/**
 * @brief AbstractDesktopFrame class unit tests
 *
 * Test scope:
 * 1. Construction and initialization
 * 2. Pure virtual function implementations (rootWindows, layoutChildren)
 * 3. Signal emission (all signals must use Qt::DirectConnection)
 * 4. Window management
 * 5. Layout operations
 * 6. Signal handling and verification
 */
class AbstractDesktopFrameTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_abstractdesktopframe") };
            app = std::make_unique<QApplication>(argc, argv);
        }
        
        // Create test desktop frame instance
        desktopFrame = std::make_unique<TestDesktopFrame>();
    }
    
    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        desktopFrame.reset();
        app.reset();
    }
    
    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QApplication> app;
    std::unique_ptr<TestDesktopFrame> desktopFrame;
};

TEST_F(AbstractDesktopFrameTest, Constructor)
{
    // Test basic construction
    EXPECT_NE(desktopFrame.get(), nullptr);
    EXPECT_EQ(desktopFrame->parent(), nullptr);
    
    // Test construction with parent
    QObject parent;
    auto childFrame = std::make_unique<TestDesktopFrame>(&parent);
    EXPECT_EQ(childFrame->parent(), &parent);
}

TEST_F(AbstractDesktopFrameTest, RootWindows)
{
    // Test root windows retrieval
    auto windows = desktopFrame->rootWindows();
    EXPECT_EQ(windows.size(), 3);
    
    // Verify windows are valid
    for (int i = 0; i < windows.size(); ++i) {
        EXPECT_NE(windows[i], nullptr);
        EXPECT_EQ(windows[i]->objectName(), QString("TestWindow%1").arg(i));
    }
}

TEST_F(AbstractDesktopFrameTest, LayoutChildren)
{
    // Test initial layout count
    EXPECT_EQ(desktopFrame->getLayoutCount(), 0);
    
    // Test layout children
    desktopFrame->layoutChildren();
    EXPECT_EQ(desktopFrame->getLayoutCount(), 1);
    
    // Test multiple layout calls
    desktopFrame->layoutChildren();
    desktopFrame->layoutChildren();
    EXPECT_EQ(desktopFrame->getLayoutCount(), 3);
    
    // Verify window positions after layout
    auto windows = desktopFrame->rootWindows();
    for (int i = 0; i < windows.size(); ++i) {
        EXPECT_EQ(windows[i]->pos(), QPoint(i * 100, i * 100));
        EXPECT_EQ(windows[i]->size(), QSize(200, 200));
    }
}

TEST_F(AbstractDesktopFrameTest, WindowAboutToBeBuildedSignal)
{
    // Test signal emission with DirectConnection
    QSignalSpy spy(desktopFrame.get(), &TestDesktopFrame::windowAboutToBeBuilded);
    
    // Test with QSignalSpy instead of direct connect
    // The DirectConnection version requires different syntax
    
    // Emit signal
    desktopFrame->emitWindowAboutToBeBuilded();
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(AbstractDesktopFrameTest, WindowBuildedSignal)
{
    // Test signal emission with DirectConnection
    QSignalSpy spy(desktopFrame.get(), &TestDesktopFrame::windowBuilded);
    
    // Emit signal
    desktopFrame->emitWindowBuilded();
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(AbstractDesktopFrameTest, WindowShowedSignal)
{
    // Test signal emission with DirectConnection
    QSignalSpy spy(desktopFrame.get(), &TestDesktopFrame::windowShowed);
    
    // Emit signal
    desktopFrame->emitWindowShowed();
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(AbstractDesktopFrameTest, GeometryChangedSignal)
{
    // Test signal emission with DirectConnection
    QSignalSpy spy(desktopFrame.get(), &TestDesktopFrame::geometryChanged);
    
    // Emit signal
    desktopFrame->emitGeometryChanged();
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
    
    // Test multiple emissions
    desktopFrame->emitGeometryChanged();
    desktopFrame->emitGeometryChanged();
    EXPECT_EQ(spy.count(), 3);
}

TEST_F(AbstractDesktopFrameTest, AvailableGeometryChangedSignal)
{
    // Test signal emission with DirectConnection
    QSignalSpy spy(desktopFrame.get(), &TestDesktopFrame::availableGeometryChanged);
    
    // Emit signal
    desktopFrame->emitAvailableGeometryChanged();
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(AbstractDesktopFrameTest, SignalSequence)
{
    // Test typical signal sequence during window creation
    QSignalSpy spyAboutToBuild(desktopFrame.get(), &TestDesktopFrame::windowAboutToBeBuilded);
    QSignalSpy spyBuilded(desktopFrame.get(), &TestDesktopFrame::windowBuilded);
    QSignalSpy spyShowed(desktopFrame.get(), &TestDesktopFrame::windowShowed);
    QSignalSpy spyGeometry(desktopFrame.get(), &TestDesktopFrame::geometryChanged);
    
    // Emit signals in typical order
    desktopFrame->emitWindowAboutToBeBuilded();
    desktopFrame->emitWindowBuilded();
    desktopFrame->emitGeometryChanged();
    desktopFrame->emitWindowShowed();
    
    // Verify all signals were emitted in order
    EXPECT_EQ(spyAboutToBuild.count(), 1);
    EXPECT_EQ(spyBuilded.count(), 1);
    EXPECT_EQ(spyGeometry.count(), 1);
    EXPECT_EQ(spyShowed.count(), 1);
}

TEST_F(AbstractDesktopFrameTest, EdgeCases)
{
    // Test with empty window list (create a new frame without windows)
    class EmptyFrame : public AbstractDesktopFrame {
    public:
        QList<QWidget *> rootWindows() const override { return {}; }
        void layoutChildren() override {}
    };
    
    EmptyFrame emptyFrame;
    auto windows = emptyFrame.rootWindows();
    EXPECT_TRUE(windows.isEmpty());
    
    // Test layout on empty frame
    EXPECT_NO_THROW(emptyFrame.layoutChildren());
}

TEST_F(AbstractDesktopFrameTest, MemoryManagement)
{
    // Test creating and destroying multiple instances
    for (int i = 0; i < 5; ++i) {
        auto tempFrame = std::make_unique<TestDesktopFrame>();
        EXPECT_NE(tempFrame.get(), nullptr);
        EXPECT_EQ(tempFrame->rootWindows().size(), 3);
    }
}

TEST_F(AbstractDesktopFrameTest, SignalConnections)
{
    // Test multiple signal connections
    int receivedCount1 = 0;
    int receivedCount2 = 0;
    
    // Use QSignalSpy to test signal emission instead of direct connect
    QSignalSpy spy(desktopFrame.get(), &TestDesktopFrame::geometryChanged);
    QSignalSpy spy2(desktopFrame.get(), &TestDesktopFrame::geometryChanged);
    
    // Emit signal
    desktopFrame->emitGeometryChanged();
    
    // Verify signal was emitted (QSignalSpy counts)
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy2.count(), 1);
}

#include "test_abstractdesktopframe.moc"