// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screenproxy_sortfilter.cpp
 * @brief Unit tests for AbstractScreenProxy (interfaces/screen/abstractscreenproxy.cpp)
 *        and AbstractSortFilter (interfaces/abstractsortfilter.cpp) via minimal
 *        concrete subclasses. AbstractScreenProxy: ctor, lastChangedMode,
 *        appendEvent, validateEvent. AbstractSortFilter: ctor, lessThan,
 *        checkFilters.
 */

#include <gtest/gtest.h>

#include <dfm-base/interfaces/screen/abstractscreenproxy.h>
#include <dfm-base/interfaces/abstractsortfilter.h>

using namespace dfmbase;

namespace {
class FakeScreenProxy : public AbstractScreenProxy
{
public:
    ScreenPointer primaryScreen() override { return nullptr; }
    QList<ScreenPointer> screens() const override { return {}; }
    QList<ScreenPointer> logicScreens() const override { return {}; }
    ScreenPointer screen(const QString &) const override { return nullptr; }
    qreal devicePixelRatio() const override { return 1.0; }
    DisplayMode displayMode() const override { return DisplayMode::kCustom; }
    void reset() override {}
protected:
    void processEvent() override {}
};
}   // namespace

TEST(AbstractScreenProxyTest, CtorAndLastChangedMode)
{
    FakeScreenProxy proxy;
    EXPECT_EQ(proxy.lastChangedMode(), DisplayMode::kCustom);
}

TEST(AbstractScreenProxyTest, AppendEventDoesNotCrash)
{
    FakeScreenProxy proxy;
    EXPECT_NO_FATAL_FAILURE({ proxy.appendEvent(AbstractScreenProxy::kScreen); });
    EXPECT_NO_FATAL_FAILURE({ proxy.appendEvent(AbstractScreenProxy::kMode); });
    EXPECT_NO_FATAL_FAILURE({ proxy.appendEvent(AbstractScreenProxy::kGeometry); });
}

TEST(AbstractScreenProxyTest, ValidateEventReturnsTrue)
{
    FakeScreenProxy proxy;
    EXPECT_TRUE(proxy.validateEvent(AbstractScreenProxy::kScreen));
    EXPECT_TRUE(proxy.validateEvent(AbstractScreenProxy::kMode));
    EXPECT_TRUE(proxy.validateEvent(AbstractScreenProxy::kGeometry));
    EXPECT_TRUE(proxy.validateEvent(AbstractScreenProxy::kAvailableGeometry));
}

TEST(AbstractSortFilterTest, LessThanReturnsMinusOne)
{
    AbstractSortFilter sf;
    FileInfoPointer left, right;
    EXPECT_EQ(sf.lessThan(left, right, false, Global::ItemRoles::kItemNameRole, static_cast<AbstractSortFilter::SortScenarios>(1)), -1);
}

TEST(AbstractSortFilterTest, CheckFiltersReturnsMinusOne)
{
    AbstractSortFilter sf;
    FileInfoPointer info;
    EXPECT_EQ(sf.checkFilters(info, QDir::NoFilter, QVariant()), -1);
}
