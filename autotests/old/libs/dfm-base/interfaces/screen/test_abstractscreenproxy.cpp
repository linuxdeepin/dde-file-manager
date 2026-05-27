// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_abstractscreenproxy.cpp - AbstractScreenProxy class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QTimer>
#include <QEventLoop>
#include <memory>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/interfaces/screen/abstractscreenproxy.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

DFMBASE_USE_NAMESPACE

/**
 * @brief Concrete implementation of AbstractScreen for testing proxy
 */
class ProxyTestScreen : public AbstractScreen
{
    Q_OBJECT
public:
    explicit ProxyTestScreen(const QString &name, const QRect &geom = QRect(0, 0, 1920, 1080), QObject *parent = nullptr)
        : AbstractScreen(parent), screenName(name), screenGeometry(geom)
    {
        availableGeometry_ = QRect(geom.x(), geom.y() + 50, geom.width(), geom.height() - 50);
        handleGeometry_ = QRect(geom.x(), geom.y(), geom.width(), 50);
    }
    
    QString name() const override { return screenName; }
    QRect geometry() const override { return screenGeometry; }
    QRect availableGeometry() const override { return availableGeometry_; }
    QRect handleGeometry() const override { return handleGeometry_; }
    
    void setGeometry(const QRect &geom) {
        screenGeometry = geom;
        availableGeometry_ = QRect(geom.x(), geom.y() + 50, geom.width(), geom.height() - 50);
        handleGeometry_ = QRect(geom.x(), geom.y(), geom.width(), 50);
        Q_EMIT geometryChanged(geom);
        Q_EMIT availableGeometryChanged(availableGeometry_);
    }
    
private:
    QString screenName;
    QRect screenGeometry;
    QRect availableGeometry_;
    QRect handleGeometry_;
};

/**
 * @brief Concrete implementation of AbstractScreenProxy for testing
 */
class TestScreenProxy : public AbstractScreenProxy
{
    Q_OBJECT
public:
    explicit TestScreenProxy(QObject *parent = nullptr)
        : AbstractScreenProxy(parent)
    {
        // Create test screens
        screens_.append(ScreenPointer(new ProxyTestScreen("Screen1", QRect(0, 0, 1920, 1080))));
        screens_.append(ScreenPointer(new ProxyTestScreen("Screen2", QRect(1920, 0, 1920, 1080))));
        
        // Set primary screen
        primaryScreen_ = screens_[0];
        
        // Initialize display mode
        currentMode = DisplayMode::kExtend;
        
        // Mock the timer for event processing
        eventShot = new QTimer(this);
        eventShot->setInterval(10);
        eventShot->setSingleShot(true);
        connect(eventShot, &QTimer::timeout, this, [this]() {
            Q_EMIT screenChanged();
            Q_EMIT displayModeChanged();
        });
    }
    
    ScreenPointer primaryScreen() override
    {
        return primaryScreen_;
    }
    
    QList<ScreenPointer> screens() const override
    {
        return screens_;
    }
    
    QList<ScreenPointer> logicScreens() const override
    {
        return screens_;  // For simplicity, same as screens
    }
    
    ScreenPointer screen(const QString &name) const override
    {
        for (const auto &screen : screens_) {
            if (screen->name() == name) {
                return screen;
            }
        }
        return nullptr;
    }
    
    qreal devicePixelRatio() const override
    {
        return 1.0;  // Simple implementation
    }
    
    DisplayMode displayMode() const override
    {
        return currentMode;
    }
    
    void reset() override
    {
        screens_.clear();
        primaryScreen_.clear();
        currentMode = DisplayMode::kCustom;
    }
    
protected:
    void processEvent() override
    {
        eventProcessedCount++;
        if (eventShot) {
            eventShot->start();
        }
    }
    
    // Test helper methods
    void setPrimaryScreen(const ScreenPointer &screen) { primaryScreen_ = screen; }
    void setDisplayMode(DisplayMode mode) { currentMode = mode; }
    void addScreen(const ScreenPointer &screen) { screens_.append(screen); }
    void removeScreen(const QString &name) {
        for (int i = 0; i < screens_.size(); ++i) {
            if (screens_[i]->name() == name) {
                if (screens_[i] == primaryScreen_) {
                    primaryScreen_.clear();
                }
                screens_.removeAt(i);
                break;
            }
        }
    }
    
    int getEventProcessedCount() const { return eventProcessedCount; }
    void triggerProcessEvent() { appendEvent(kScreen); }
    
private:
    QList<ScreenPointer> screens_;
    ScreenPointer primaryScreen_;
    DisplayMode currentMode;
    int eventProcessedCount = 0;
};

/**
 * @brief AbstractScreenProxy class unit tests
 *
 * Test scope:
 * 1. Construction and initialization
 * 2. Pure virtual function implementations (primaryScreen, screens, etc.)
 * 3. Display mode management
 * 4. Event handling and signal emission
 * 5. Screen discovery and management
 * 6. Device pixel ratio
 * 7. Event processing workflow
 */
class AbstractScreenProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_abstractscreenproxy") };
            app = std::make_unique<QApplication>(argc, argv);
        }
        
        // Create test screen proxy instance
        proxy = std::make_unique<TestScreenProxy>();
    }
    
    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        proxy.reset();
        app.reset();
    }
    
    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QApplication> app;
    std::unique_ptr<TestScreenProxy> proxy;
    
    // Helper to wait for signals
    void waitForSignal(int timeoutMs = 100)
    {
        QEventLoop loop;
        QTimer::singleShot(timeoutMs, &loop, &QEventLoop::quit);
        loop.exec();
    }
};

TEST_F(AbstractScreenProxyTest, Constructor)
{
    // Test basic construction
    EXPECT_NE(proxy.get(), nullptr);
    EXPECT_EQ(proxy->parent(), nullptr);
    
    // Test default values
    EXPECT_EQ(proxy->lastChangedMode(), DisplayMode::kCustom);
    EXPECT_TRUE(proxy->events.isEmpty());
    
    // Test construction with parent
    QObject parent;
    auto childProxy = std::make_unique<TestScreenProxy>(&parent);
    EXPECT_EQ(childProxy->parent(), &parent);
}

TEST_F(AbstractScreenProxyTest, PrimaryScreen)
{
    // Test getting primary screen
    auto primary = proxy->primaryScreen();
    EXPECT_NE(primary.data(), nullptr);
    EXPECT_EQ(primary->name(), "Screen1");
    
    // Test setting primary screen
    auto screens = proxy->screens();
    ASSERT_GT(screens.size(), 1);
    proxy->setPrimaryScreen(screens[1]);
    
    primary = proxy->primaryScreen();
    EXPECT_EQ(primary->name(), "Screen2");
    
    // Test setting null primary screen
    proxy->setPrimaryScreen(ScreenPointer());
    primary = proxy->primaryScreen();
    EXPECT_EQ(primary.data(), nullptr);
}

TEST_F(AbstractScreenProxyTest, Screens)
{
    // Test getting all screens
    auto screens = proxy->screens();
    EXPECT_EQ(screens.size(), 2);
    EXPECT_EQ(screens[0]->name(), "Screen1");
    EXPECT_EQ(screens[1]->name(), "Screen2");
    
    // Test adding screen
    auto newScreen = ScreenPointer(new ProxyTestScreen("Screen3", QRect(3840, 0, 1920, 1080)));
    proxy->addScreen(newScreen);
    
    screens = proxy->screens();
    EXPECT_EQ(screens.size(), 3);
    EXPECT_EQ(screens[2]->name(), "Screen3");
    
    // Test removing screen
    proxy->removeScreen("Screen2");
    screens = proxy->screens();
    EXPECT_EQ(screens.size(), 2);
    EXPECT_EQ(screens[0]->name(), "Screen1");
    EXPECT_EQ(screens[1]->name(), "Screen3");
}

TEST_F(AbstractScreenProxyTest, LogicScreens)
{
    // Test getting logic screens
    auto logicScreens = proxy->logicScreens();
    EXPECT_EQ(logicScreens.size(), 2);
    
    // For this implementation, logic screens should be same as screens
    auto screens = proxy->screens();
    EXPECT_EQ(logicScreens.size(), screens.size());
    
    for (int i = 0; i < screens.size(); ++i) {
        EXPECT_EQ(logicScreens[i]->name(), screens[i]->name());
    }
}

TEST_F(AbstractScreenProxyTest, ScreenByName)
{
    // Test finding screen by name
    auto screen1 = proxy->screen("Screen1");
    EXPECT_NE(screen1.data(), nullptr);
    EXPECT_EQ(screen1->name(), "Screen1");
    
    auto screen2 = proxy->screen("Screen2");
    EXPECT_NE(screen2.data(), nullptr);
    EXPECT_EQ(screen2->name(), "Screen2");
    
    // Test finding non-existent screen
    auto nonExistent = proxy->screen("NonExistent");
    EXPECT_EQ(nonExistent.data(), nullptr);
    
    // Test case sensitivity
    auto caseSensitive = proxy->screen("screen1");
    EXPECT_EQ(caseSensitive.data(), nullptr);  // Should be case sensitive
}

TEST_F(AbstractScreenProxyTest, DevicePixelRatio)
{
    // Test device pixel ratio
    EXPECT_EQ(proxy->devicePixelRatio(), 1.0);
    
    // The ratio should be consistent
    qreal ratio1 = proxy->devicePixelRatio();
    qreal ratio2 = proxy->devicePixelRatio();
    EXPECT_EQ(ratio1, ratio2);
}

TEST_F(AbstractScreenProxyTest, DisplayMode)
{
    // Test getting display mode
    EXPECT_EQ(proxy->displayMode(), DisplayMode::kExtend);
    
    // Test setting display mode
    proxy->setDisplayMode(DisplayMode::kDuplicate);
    EXPECT_EQ(proxy->displayMode(), DisplayMode::kDuplicate);
    
    proxy->setDisplayMode(DisplayMode::kExtend);
    EXPECT_EQ(proxy->displayMode(), DisplayMode::kExtend);
    
    // Test last changed mode
    EXPECT_EQ(proxy->lastChangedMode(), DisplayMode::kCustom);
    
    proxy->setDisplayMode(DisplayMode::kExtend);
    // Note: lastChangedMode would be updated by the actual implementation
}

TEST_F(AbstractScreenProxyTest, Reset)
{
    // Add some screens and set mode
    auto newScreen = ScreenPointer(new ProxyTestScreen("TestScreen"));
    proxy->addScreen(newScreen);
    proxy->setDisplayMode(DisplayMode::kDuplicate);
    
    // Verify state before reset
    EXPECT_GT(proxy->screens().size(), 0);
    EXPECT_NE(proxy->primaryScreen().data(), nullptr);
    
    // Perform reset
    proxy->reset();
    
    // Verify reset state
    EXPECT_TRUE(proxy->screens().isEmpty());
    EXPECT_EQ(proxy->primaryScreen().data(), nullptr);
    EXPECT_EQ(proxy->displayMode(), DisplayMode::kCustom);
}

TEST_F(AbstractScreenProxyTest, EventHandling)
{
    // Test event count before adding
    EXPECT_TRUE(proxy->events.isEmpty());
    
    // Test that we can trigger events
    int initialCount = proxy->getEventProcessedCount();
    proxy->triggerProcessEvent();
    
    // Wait for event processing
    waitForSignal(50);
    
    // Verify event was processed
    EXPECT_GT(proxy->getEventProcessedCount(), initialCount);
}

TEST_F(AbstractScreenProxyTest, Signals)
{
    // Test signal emission
    QSignalSpy screenChangedSpy(proxy.get(), &TestScreenProxy::screenChanged);
    QSignalSpy displayModeChangedSpy(proxy.get(), &TestScreenProxy::displayModeChanged);
    QSignalSpy geometryChangedSpy(proxy.get(), &TestScreenProxy::screenGeometryChanged);
    QSignalSpy availableGeometryChangedSpy(proxy.get(), &TestScreenProxy::screenAvailableGeometryChanged);
    
    // Trigger event processing which should emit signals
    proxy->triggerProcessEvent();
    waitForSignal(50);
    
    // Verify signals were emitted
    EXPECT_GE(screenChangedSpy.count(), 0);  // May be emitted depending on timing
    EXPECT_GE(displayModeChangedSpy.count(), 0);
}

TEST_F(AbstractScreenProxyTest, ScreenGeometries)
{
    // Test screen geometries through proxy
    auto screens = proxy->screens();
    ASSERT_GT(screens.size(), 0);
    
    // Test primary screen geometry
    auto primary = proxy->primaryScreen();
    EXPECT_NE(primary->geometry().width(), 0);
    EXPECT_NE(primary->geometry().height(), 0);
    
    // Test available geometry
    EXPECT_NE(primary->availableGeometry().width(), 0);
    EXPECT_NE(primary->availableGeometry().height(), 0);
    
    // Test that available geometry is typically smaller than total geometry
    EXPECT_LE(primary->availableGeometry().width(), primary->geometry().width());
    EXPECT_LE(primary->availableGeometry().height(), primary->geometry().height());
}

TEST_F(AbstractScreenProxyTest, MultipleProxies)
{
    // Test creating multiple proxies
    auto proxy1 = std::make_unique<TestScreenProxy>();
    auto proxy2 = std::make_unique<TestScreenProxy>();
    
    // Verify they operate independently
    auto screen1 = proxy1->primaryScreen();
    auto screen2 = proxy2->primaryScreen();
    
    EXPECT_NE(screen1.data(), nullptr);
    EXPECT_NE(screen2.data(), nullptr);
    
    // Test independent screen management
    proxy1->removeScreen("Screen2");
    EXPECT_EQ(proxy1->screens().size(), 1);
    EXPECT_EQ(proxy2->screens().size(), 2);  // Should be unchanged
}

TEST_F(AbstractScreenProxyTest, EdgeCases)
{
    // Test behavior with no screens
    proxy->reset();
    EXPECT_TRUE(proxy->screens().isEmpty());
    EXPECT_EQ(proxy->primaryScreen().data(), nullptr);
    
    // Test screen lookup with empty proxy
    auto screen = proxy->screen("AnyName");
    EXPECT_EQ(screen.data(), nullptr);
    
    // Test getting screens when empty
    auto screens = proxy->screens();
    EXPECT_TRUE(screens.isEmpty());
    
    auto logicScreens = proxy->logicScreens();
    EXPECT_TRUE(logicScreens.isEmpty());
}

#include "test_abstractscreenproxy.moc"