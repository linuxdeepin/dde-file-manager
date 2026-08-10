// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractscreen.cpp
 * @brief Unit tests for AbstractScreen base class (abstractscreen.cpp)
 *
 * AbstractScreen is an abstract QObject base for screen representations. Its
 * only non-pure member is the constructor; the geometry accessors are pure
 * virtual and are meant to be implemented by concrete (hardware-backed or
 * stub) subclasses. There is no real display device available in the unit test
 * environment, so a small TestableScreen stub provides deterministic values
 * and lets us exercise the base class, including its Q_OBJECT signals, fully
 * without any hardware.
 */

#include <gtest/gtest.h>
#include <dfm-base/interfaces/screen/abstractscreen.h>

#include <QSignalSpy>
#include <QRect>
#include <QObject>

using namespace dfmbase;

namespace {
// Concrete stub subclass: implements the pure virtuals with settable values
// and exposes helpers to emit the inherited signals. It needs no Q_OBJECT of
// its own because it reuses AbstractScreen's meta-object for the signals.
class TestableScreen : public AbstractScreen
{
public:
    explicit TestableScreen(QObject *parent = nullptr)
        : AbstractScreen(parent) {}

    QString m_name = QStringLiteral("screen-0");
    QRect m_geometry { 0, 0, 1920, 1080 };
    QRect m_available { 0, 0, 1920, 1040 };
    QRect m_handle { 0, 0, 1920, 1080 };

    QString name() const override { return m_name; }
    QRect geometry() const override { return m_geometry; }
    QRect availableGeometry() const override { return m_available; }
    QRect handleGeometry() const override { return m_handle; }
    bool checkAvailableGeometry(const QRect &available, const QRect &screen) const override
    {
        return screen.contains(available);
    }

    void emitGeometryChanged(const QRect &r) { emit geometryChanged(r); }
    void emitAvailableGeometryChanged(const QRect &r) { emit availableGeometryChanged(r); }
};
}   // namespace

TEST(AbstractScreenTest, ConstructorSetsParent)
{
    QObject parent;
    AbstractScreen *screen = new TestableScreen(&parent);
    EXPECT_EQ(screen->parent(), &parent);
    delete screen;
}

TEST(AbstractScreenTest, ConstructorAcceptsNullParent)
{
    AbstractScreen *screen = new TestableScreen(nullptr);
    EXPECT_EQ(screen->parent(), nullptr);
    delete screen;
}

TEST(AbstractScreenTest, NameAndGeometryAccessorsReturnConfiguredValues)
{
    TestableScreen screen;
    screen.m_name = QStringLiteral("HDMI-1");
    screen.m_geometry = QRect(10, 20, 1280, 720);
    screen.m_available = QRect(10, 20, 1280, 700);
    screen.m_handle = QRect(0, 0, 1280, 720);

    EXPECT_EQ(screen.name(), QStringLiteral("HDMI-1"));
    EXPECT_EQ(screen.geometry(), QRect(10, 20, 1280, 720));
    EXPECT_EQ(screen.availableGeometry(), QRect(10, 20, 1280, 700));
    EXPECT_EQ(screen.handleGeometry(), QRect(0, 0, 1280, 720));
}

TEST(AbstractScreenTest, CheckAvailableGeometryDelegatesToStubImplementation)
{
    TestableScreen screen;
    // available fully inside screen -> true
    EXPECT_TRUE(screen.checkAvailableGeometry(QRect(10, 10, 100, 100), QRect(0, 0, 1920, 1080)));
    // available sticks out of screen -> false
    EXPECT_FALSE(screen.checkAvailableGeometry(QRect(0, 0, 2000, 1080), QRect(0, 0, 1920, 1080)));
}

TEST(AbstractScreenTest, GeometryChangedSignalPropagatesValue)
{
    TestableScreen screen;
    QSignalSpy spy(&screen, &AbstractScreen::geometryChanged);
    QRect emitted(1, 2, 3, 4);
    screen.emitGeometryChanged(emitted);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toRect(), emitted);
}

TEST(AbstractScreenTest, AvailableGeometryChangedSignalPropagatesValue)
{
    TestableScreen screen;
    QSignalSpy spy(&screen, &AbstractScreen::availableGeometryChanged);
    QRect emitted(5, 6, 7, 8);
    screen.emitAvailableGeometryChanged(emitted);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toRect(), emitted);
}
