// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractdesktopframe.cpp
 * @brief Unit tests for AbstractDesktopFrame base class (abstractdesktopframe.cpp)
 *
 * AbstractDesktopFrame is an abstract QObject base for desktop frame
 * implementations. Only the constructor has a .cpp body; the geometry
 * accessors are pure virtual and the five Q_OBJECT signals are meant to be
 * emitted by concrete subclasses. A small TestableFrame stub provides
 * deterministic implementations and helpers to emit the inherited signals,
 * exercising the base class fully without any real screen hardware.
 */

#include <gtest/gtest.h>
#include <dfm-base/interfaces/abstractdesktopframe.h>

#include <QSignalSpy>
#include <QWidget>
#include <QObject>
#include <QList>

using namespace dfmbase;

namespace {
class TestableFrame : public AbstractDesktopFrame
{
public:
    explicit TestableFrame(QObject *parent = nullptr)
        : AbstractDesktopFrame(parent) {}

    QList<QWidget *> m_rootWindows;
    int m_layoutCalls = 0;

    QList<QWidget *> rootWindows() const override { return m_rootWindows; }
    void layoutChildren() override { ++m_layoutCalls; }

    void emitWindowAboutToBeBuilded() { emit windowAboutToBeBuilded(); }
    void emitWindowBuilded() { emit windowBuilded(); }
    void emitWindowShowed() { emit windowShowed(); }
    void emitGeometryChanged() { emit geometryChanged(); }
    void emitAvailableGeometryChanged() { emit availableGeometryChanged(); }
};
}   // namespace

TEST(AbstractDesktopFrameTest, ConstructorSetsParent)
{
    QObject parent;
    auto *frame = new TestableFrame(&parent);
    EXPECT_EQ(frame->parent(), &parent);
    delete frame;
}

TEST(AbstractDesktopFrameTest, ConstructorAcceptsNullParent)
{
    auto *frame = new TestableFrame(nullptr);
    EXPECT_EQ(frame->parent(), nullptr);
    delete frame;
}

TEST(AbstractDesktopFrameTest, RootWindowsReturnsConfiguredList)
{
    TestableFrame frame;
    QWidget w1, w2;
    frame.m_rootWindows = { &w1, &w2 };
    auto roots = frame.rootWindows();
    EXPECT_EQ(roots.size(), 2);
    EXPECT_EQ(roots.at(0), &w1);
    EXPECT_EQ(roots.at(1), &w2);
}

TEST(AbstractDesktopFrameTest, LayoutChildrenInvokedOnCall)
{
    TestableFrame frame;
    EXPECT_EQ(frame.m_layoutCalls, 0);
    frame.layoutChildren();
    frame.layoutChildren();
    EXPECT_EQ(frame.m_layoutCalls, 2);
}

TEST(AbstractDesktopFrameTest, AllFiveSignalsPropagate)
{
    TestableFrame frame;
    QSignalSpy aboutSpy(&frame, &AbstractDesktopFrame::windowAboutToBeBuilded);
    QSignalSpy buildedSpy(&frame, &AbstractDesktopFrame::windowBuilded);
    QSignalSpy showedSpy(&frame, &AbstractDesktopFrame::windowShowed);
    QSignalSpy geomSpy(&frame, &AbstractDesktopFrame::geometryChanged);
    QSignalSpy availSpy(&frame, &AbstractDesktopFrame::availableGeometryChanged);

    frame.emitWindowAboutToBeBuilded();
    frame.emitWindowBuilded();
    frame.emitWindowShowed();
    frame.emitGeometryChanged();
    frame.emitAvailableGeometryChanged();

    EXPECT_EQ(aboutSpy.count(), 1);
    EXPECT_EQ(buildedSpy.count(), 1);
    EXPECT_EQ(showedSpy.count(), 1);
    EXPECT_EQ(geomSpy.count(), 1);
    EXPECT_EQ(availSpy.count(), 1);
}
