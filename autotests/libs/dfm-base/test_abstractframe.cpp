// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractframe.cpp
 * @brief Unit tests for AbstractFrame (abstractframe.cpp)
 */

#include <gtest/gtest.h>
#include <dfm-base/interfaces/abstractframe.h>

#include <QUrl>

using namespace dfmbase;

namespace {
class TestableFrame : public AbstractFrame
{
public:
    explicit TestableFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : AbstractFrame(parent, f) {}

    QUrl m_url;
    void setCurrentUrl(const QUrl &url) override { m_url = url; }
    QUrl currentUrl() const override { return m_url; }
};
}   // namespace

TEST(AbstractFrameTest, ConstructDoesNotCrash)
{
    TestableFrame f;
    EXPECT_FALSE(f.currentUrl().isValid());
}

TEST(AbstractFrameTest, SetCurrentUrlReturnsSameUrl)
{
    TestableFrame f;
    QUrl url(QStringLiteral("file:///tmp"));
    f.setCurrentUrl(url);
    EXPECT_EQ(f.currentUrl(), url);
}

TEST(AbstractFrameTest, ConstructWithParentDoesNotCrash)
{
    QWidget parent;
    auto *f = new TestableFrame(&parent);
    EXPECT_EQ(f->parent(), &parent);
    delete f;
}
