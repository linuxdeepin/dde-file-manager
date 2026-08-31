// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenuprivate_1.cpp
 * @brief Unit tests for OemMenuPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "oemmenuscene/oemmenu.h"

#include <QTest>

using namespace dfmplugin_menu;

class OemMenuPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OemMenuPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OemMenuPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OemMenuPrivateTest, OemMenuPrivate)
{
    // Test constructor: OemMenuPrivate((OemMenu *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OemMenuPrivateTest, isActionShouldShow)
{
    // Test method: bool isActionShouldShow((const QAction *action, bool onDesktop))
    auto result = obj->isActionShouldShow(nullptr, false);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuPrivateTest, isAllEx7zFile)
{
    // Test method: bool isAllEx7zFile((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    auto result = obj->isAllEx7zFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuPrivateTest, isSchemeSupport)
{
    // Test method: bool isSchemeSupport((const QAction *action, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->isSchemeSupport(nullptr, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuPrivateTest, replaceList)
{
    // Test method: QStringList replaceList((QStringList &args, const QString &before, const QStringList &after))
    QStringList _arg0{};
    QString _arg1{};
    QStringList _arg2{};
    auto result = obj->replaceList(_arg0, _arg1, _arg2);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuPrivateTest, setActionProperty)
{
    // Test setter: void setActionProperty((QAction *const action, const DDesktopEntry &entry, const QString &key, const QString &section))
    DDesktopEntry _arg1{};
    QString _arg2{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->setActionProperty(nullptr, _arg1, _arg2, _arg3));
}

TEST_F(OemMenuPrivateTest, urlListToLocalFile)
{
    // Test method: QStringList urlListToLocalFile((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    auto result = obj->urlListToLocalFile(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuPrivateTest, urlListToString)
{
    // Test method: QStringList urlListToString((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    auto result = obj->urlListToString(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuPrivateTest, urlToString)
{
    // Test method: QString urlToString((const QUrl &file))
    QUrl _arg0{};
    auto result = obj->urlToString(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuPrivateTest, OemMenuPrivate_Destructor)
{
    // Test method:  ~OemMenuPrivate(())
    EXPECT_NO_FATAL_FAILURE({ OemMenuPrivate *tmp = new OemMenuPrivate(); delete tmp; });
}
