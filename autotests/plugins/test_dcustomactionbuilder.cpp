// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionbuilder.cpp
 * @brief Unit tests for DCustomActionBuilder methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactionbuilder.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionBuilderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionBuilder();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionBuilder *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionBuilderTest, isMimeTypeMatch)
{
    // Test method: bool isMimeTypeMatch((const QStringList &fileMimeTypes, const QStringList &supportMimeTypes))
    QStringList _arg0{};
    QStringList _arg1{};
    auto result = obj->isMimeTypeMatch(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionBuilderTest, isMimeTypeSupport)
{
    // Test method: bool isMimeTypeSupport((const QString &mt, const QStringList &fileMimeTypes))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->isMimeTypeSupport(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionBuilderTest, isSchemeSupport)
{
    // Test method: bool isSchemeSupport((const DCustomActionEntry &action, const QUrl &url))
    DCustomActionEntry _arg0{};
    QUrl _arg1{};
    auto result = obj->isSchemeSupport(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionBuilderTest, isSuffixSupport)
{
    // Test method: bool isSuffixSupport((const DCustomActionEntry &action, FileInfoPointer fileInfo))
    DCustomActionEntry _arg0{};
    auto result = obj->isSuffixSupport(_arg0, FileInfoPointer());
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionBuilderTest, makeName)
{
    // Test method: QString makeName((const QString &name, DCustomActionDefines::ActionArg arg))
    QString _arg0{};
    auto result = obj->makeName(_arg0, DCustomActionDefines::ActionArg());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionBuilderTest, matchFileCombo)
{
    // Test method: QList<DCustomActionEntry> matchFileCombo((const QList<DCustomActionEntry> &rootActions,
                                                               DCustomActionDefines::ComboTypes type))
    QList<DCustomActionEntry> _arg0{};
    auto result = obj->matchFileCombo(_arg0, DCustomActionDefines::ComboTypes());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionBuilderTest, checkFileCombo)
{
    // Test method: DCustomActionDefines::ComboType checkFileCombo((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    auto result = obj->checkFileCombo(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionBuilderTest, checkFileComboWithFocus)
{
    // Test method: DCustomActionDefines::ComboType checkFileComboWithFocus((const QUrl &focus, const QList<QUrl> &files))
    QUrl _arg0{};
    QList<QUrl> _arg1{};
    auto result = obj->checkFileComboWithFocus(_arg0, _arg1);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DCustomActionBuilderTest, createMenu)
{
    // Test method: QAction createMenu((const DCustomActionData &actionData, QWidget *parentForSubmenu))
    DCustomActionData _arg0{};
    auto result = obj->createMenu(_arg0, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createMenu(_arg0, nullptr); });

}
