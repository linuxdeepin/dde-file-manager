// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// test_abstractscreen.cpp - AbstractScreen class unit tests
// Using stub_ext for function stubbing

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QApplication>
#include <QRect>
#include <memory>

// Include stub_ext
#include "stubext.h"

// Include test target classes
#include <dfm-base/interfaces/screen/abstractscreen.h>

DFMBASE_USE_NAMESPACE

/**
 * @brief Concrete implementation of AbstractScreen for testing
 */
class TestScreen : public AbstractScreen
{
    Q_OBJECT
public:
    explicit TestScreen(const QString &screenName = "TestScreen", QObject *parent = nullptr)
        : AbstractScreen(parent), screenName(screenName)
    {
        // Initialize with default geometry
        geom = QRect(0, 0, 1920, 1080);
        availableGeom = QRect(0, 50, 1920, 1030);  // Account for menu bar
        handleGeom = QRect(0, 0, 1920, 50);  // Menu bar area
    }
    
    QString name() const override
    {
        return screenName;
    }
    
    QRect geometry() const override
    {
        return geom;
    }
    
    QRect availableGeometry() const override
    {
        return availableGeom;
    }
    
    QRect handleGeometry() const override
    {
        return handleGeom;
    }
    
    // Test helper methods
    void setScreenName(const QString &name) { screenName = name; }
    void setGeometry(const QRect &rect) { 
        geom = rect;
        Q_EMIT geometryChanged(rect);
    }
    void setAvailableGeometry(const QRect &rect) { 
        availableGeom = rect;
        Q_EMIT availableGeometryChanged(rect);
    }
    void setHandleGeometry(const QRect &rect) { handleGeom = rect; }
    
private:
    QString screenName;
    QRect geom;
    QRect availableGeom;
    QRect handleGeom;
};

/**
 * @brief AbstractScreen class unit tests
 *
 * Test scope:
 * 1. Construction and initialization
 * 2. Pure virtual function implementations (name, geometry methods)
 * 3. Signal emission (geometryChanged, availableGeometryChanged)
 * 4. Screen property management
 * 5. Geometry calculations and relationships
 * 6. ScreenPointer usage and shared pointer behavior
 * 7. Edge cases and boundary conditions
 */
class AbstractScreenTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        stub.clear();
        
        // Set up test application if not exists
        if (!qApp) {
            static int argc = 1;
            static char *argv[] = { const_cast<char*>("test_abstractscreen") };
            app = std::make_unique<QApplication>(argc, argv);
        }
        
        // Create test screen instance
        screen = std::make_unique<TestScreen>();
    }
    
    void TearDown() override
    {
        // Clean up test environment
        stub.clear();
        screen.reset();
        app.reset();
    }
    
    // Test stubbing utility
    stub_ext::StubExt stub;
    std::unique_ptr<QApplication> app;
    std::unique_ptr<TestScreen> screen;
};

TEST_F(AbstractScreenTest, Constructor)
{
    // Test basic construction
    EXPECT_NE(screen.get(), nullptr);
    EXPECT_EQ(screen->parent(), nullptr);
    
    // Test default properties
    EXPECT_EQ(screen->name(), "TestScreen");
    EXPECT_EQ(screen->geometry(), QRect(0, 0, 1920, 1080));
    EXPECT_EQ(screen->availableGeometry(), QRect(0, 50, 1920, 1030));
    EXPECT_EQ(screen->handleGeometry(), QRect(0, 0, 1920, 50));
    
    // Test construction with parent
    QObject parent;
    auto childScreen = std::make_unique<TestScreen>("ChildScreen", &parent);
    EXPECT_EQ(childScreen->parent(), &parent);
    EXPECT_EQ(childScreen->name(), "ChildScreen");
}

TEST_F(AbstractScreenTest, ScreenName)
{
    // Test getting screen name
    EXPECT_EQ(screen->name(), "TestScreen");
    
    // Test setting screen name
    screen->setScreenName("NewScreenName");
    EXPECT_EQ(screen->name(), "NewScreenName");
    
    // Test with special characters
    screen->setScreenName("Screen-123_测试");
    EXPECT_EQ(screen->name(), "Screen-123_测试");
    
    // Test with empty string
    screen->setScreenName("");
    EXPECT_EQ(screen->name(), "");
}

TEST_F(AbstractScreenTest, Geometry)
{
    // Test default geometry
    EXPECT_EQ(screen->geometry(), QRect(0, 0, 1920, 1080));
    
    // Test setting geometry
    QRect newGeom(100, 100, 2560, 1440);
    screen->setGeometry(newGeom);
    EXPECT_EQ(screen->geometry(), newGeom);
    
    // Test with zero-sized geometry
    QRect zeroGeom(0, 0, 0, 0);
    screen->setGeometry(zeroGeom);
    EXPECT_EQ(screen->geometry(), zeroGeom);
    
    // Test with negative coordinates
    QRect negativeGeom(-100, -100, 800, 600);
    screen->setGeometry(negativeGeom);
    EXPECT_EQ(screen->geometry(), negativeGeom);
}

TEST_F(AbstractScreenTest, AvailableGeometry)
{
    // Test default available geometry
    EXPECT_EQ(screen->availableGeometry(), QRect(0, 50, 1920, 1030));
    
    // Test setting available geometry
    QRect newAvailGeom(0, 80, 1920, 1000);
    screen->setAvailableGeometry(newAvailGeom);
    EXPECT_EQ(screen->availableGeometry(), newAvailGeom);
    
    // Test that available geometry can be different from geometry
    screen->setGeometry(QRect(0, 0, 1920, 1080));
    screen->setAvailableGeometry(QRect(0, 100, 1920, 980));
    EXPECT_NE(screen->geometry(), screen->availableGeometry());
}

TEST_F(AbstractScreenTest, HandleGeometry)
{
    // Test default handle geometry
    EXPECT_EQ(screen->handleGeometry(), QRect(0, 0, 1920, 50));
    
    // Test setting handle geometry
    QRect newHandleGeom(0, 0, 1920, 80);
    screen->setHandleGeometry(newHandleGeom);
    EXPECT_EQ(screen->handleGeometry(), newHandleGeom);
    
    // Test handle geometry relationship with other geometries
    screen->setGeometry(QRect(0, 0, 1920, 1080));
    screen->setHandleGeometry(QRect(0, 0, 1920, 40));
    screen->setAvailableGeometry(QRect(0, 40, 1920, 1040));
    
    EXPECT_EQ(screen->geometry().top(), 0);
    EXPECT_EQ(screen->handleGeometry().top(), 0);
    EXPECT_EQ(screen->availableGeometry().top(), 40);
}

TEST_F(AbstractScreenTest, GeometryChangedSignal)
{
    // Test geometry changed signal
    QSignalSpy spy(screen.get(), &TestScreen::geometryChanged);
    
    QRect newGeom(200, 200, 1280, 720);
    screen->setGeometry(newGeom);
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toRect(), newGeom);
    
    // Test multiple changes
    screen->setGeometry(QRect(0, 0, 800, 600));
    screen->setGeometry(QRect(100, 100, 1024, 768));
    EXPECT_EQ(spy.count(), 2);
}

TEST_F(AbstractScreenTest, AvailableGeometryChangedSignal)
{
    // Test available geometry changed signal
    QSignalSpy spy(screen.get(), &TestScreen::availableGeometryChanged);
    
    QRect newAvailGeom(0, 60, 1920, 1020);
    screen->setAvailableGeometry(newAvailGeom);
    
    // Verify signal was emitted
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toRect(), newAvailGeom);
    
    // Test multiple changes
    screen->setAvailableGeometry(QRect(0, 80, 1920, 1000));
    screen->setAvailableGeometry(QRect(0, 100, 1920, 980));
    EXPECT_EQ(spy.count(), 2);
}

TEST_F(AbstractScreenTest, SignalSequence)
{
    // Test typical signal sequence when geometry changes
    QSignalSpy geomSpy(screen.get(), &TestScreen::geometryChanged);
    QSignalSpy availGeomSpy(screen.get(), &TestScreen::availableGeometryChanged);
    
    // Change both geometries
    screen->setGeometry(QRect(0, 0, 2560, 1440));
    screen->setAvailableGeometry(QRect(0, 50, 2560, 1390));
    
    // Verify both signals were emitted
    EXPECT_EQ(geomSpy.count(), 1);
    EXPECT_EQ(availGeomSpy.count(), 1);
}

TEST_F(AbstractScreenTest, ScreenPointerUsage)
{
    // Test ScreenPointer (QSharedPointer) usage
    ScreenPointer screenPtr = ScreenPointer(new TestScreen("PointerScreen"));
    EXPECT_NE(screenPtr.data(), nullptr);
    EXPECT_EQ(screenPtr->name(), "PointerScreen");
    
    // Test shared pointer behavior
    ScreenPointer screenPtr2 = screenPtr;
    EXPECT_EQ(screenPtr.data(), screenPtr2.data());
    // Note: use_count() might not be available on all platforms/configurations
    // EXPECT_EQ(screenPtr.use_count(), 2);
    
    // Test geometry through pointer
    // Since ScreenPointer is of type AbstractScreen, we need to cast to TestScreen
    // to access the setGeometry method which is specific to our test implementation
    TestScreen *testScreen = qobject_cast<TestScreen*>(screenPtr.data());
    TestScreen *testScreen2 = qobject_cast<TestScreen*>(screenPtr2.data());
    
    if (testScreen && testScreen2) {
        testScreen->setGeometry(QRect(0, 0, 1024, 768));
        EXPECT_EQ(testScreen2->geometry(), QRect(0, 0, 1024, 768));
        
        // Test signal through pointer
        QSignalSpy spy(testScreen, &TestScreen::geometryChanged);
        testScreen2->setGeometry(QRect(100, 100, 1280, 720));
        EXPECT_EQ(spy.count(), 1);
    }
}

TEST_F(AbstractScreenTest, MultipleScreens)
{
    // Test creating multiple screen instances
    QList<ScreenPointer> screens;
    
    for (int i = 0; i < 5; ++i) {
        ScreenPointer screenPtr = ScreenPointer(new TestScreen(QString("Screen%1").arg(i)));
        screens.append(screenPtr);
        
        // Configure each screen with different geometry
        // Since ScreenPointer is of type AbstractScreen, we need to cast to TestScreen
        TestScreen *testScreen = qobject_cast<TestScreen*>(screenPtr.data());
        if (testScreen) {
            testScreen->setGeometry(QRect(i * 100, i * 100, 800, 600));
        }
    }
    
    // Verify all screens have unique properties
    for (int i = 0; i < screens.size(); ++i) {
        EXPECT_EQ(screens[i]->name(), QString("Screen%1").arg(i));
        EXPECT_EQ(screens[i]->geometry(), QRect(i * 100, i * 100, 800, 600));
    }
}

TEST_F(AbstractScreenTest, EdgeCases)
{
    // Test with very large geometry
    QRect largeGeom(0, 0, 10000, 10000);
    screen->setGeometry(largeGeom);
    EXPECT_EQ(screen->geometry(), largeGeom);
    
    // Test with negative size (should still store as is)
    QRect negativeSizeGeom(100, 100, -800, -600);
    screen->setGeometry(negativeSizeGeom);
    EXPECT_EQ(screen->geometry(), negativeSizeGeom);
    
    // Test with very small geometry
    QRect tinyGeom(5000, 5000, 1, 1);
    screen->setGeometry(tinyGeom);
    EXPECT_EQ(screen->geometry(), tinyGeom);
}

TEST_F(AbstractScreenTest, GeometryRelationships)
{
    // Test typical desktop layout relationships
    screen->setGeometry(QRect(0, 0, 1920, 1080));
    screen->setHandleGeometry(QRect(0, 0, 1920, 32));  // Top menu bar
    screen->setAvailableGeometry(QRect(0, 32, 1920, 1048));
    
    // Verify relationships
    EXPECT_TRUE(screen->geometry().contains(screen->handleGeometry()));
    EXPECT_TRUE(screen->geometry().contains(screen->availableGeometry()));
    
    // Verify handle area doesn't overlap available area
    EXPECT_FALSE(screen->handleGeometry().intersects(screen->availableGeometry()));
    
    // Test available geometry is smaller than total geometry
    auto totalGeom = screen->geometry();
    auto availGeom = screen->availableGeometry();
    EXPECT_TRUE(availGeom.width() <= totalGeom.width());
    EXPECT_TRUE(availGeom.height() <= totalGeom.height());
}

#include "test_abstractscreen.moc"