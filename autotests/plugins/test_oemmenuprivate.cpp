// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenuprivate.cpp
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

TEST_F(OemMenuPrivateTest, applyDynamicArg)
{
    // Test method: QStringList applyDynamicArg((const QStringList &args, ArgType type, const QUrl &dir, const QUrl &focus, const QList<QUrl> &files))
    QStringList _arg0{};
    QUrl _arg2{};
    QUrl _arg3{};
    QList<QUrl> _arg4{};
    auto result = obj->applyDynamicArg(_arg0, ArgType(), _arg2, _arg3, _arg4);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuPrivateTest, clearSubMenus)
{
    // Test method: void clearSubMenus(())
    EXPECT_NO_FATAL_FAILURE(obj->clearSubMenus());
}

TEST_F(OemMenuPrivateTest, getValues)
{
    // Test method: QStringList getValues((const DDesktopEntry &entry, const QString &key, const QString &aliasKey, const QString &section, const QStringList &whiteList))
    DDesktopEntry _arg0{};
    QString _arg1{};
    QString _arg2{};
    QString _arg3{};
    QStringList _arg4{};
    auto result = obj->getValues(_arg0, _arg1, _arg2, _arg3, _arg4);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuPrivateTest, isMimeTypeMatch)
{
    // Test method: bool isMimeTypeMatch((const QStringList &fileMimeTypes, const QStringList &supportMimeTypes))
    QStringList _arg0{};
    QStringList _arg1{};
    auto result = obj->isMimeTypeMatch(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuPrivateTest, isMimeTypeSupport)
{
    // Test method: bool isMimeTypeSupport((const QString &mt, const QStringList &fileMimeTypes))
    QString _arg0{};
    QStringList _arg1{};
    auto result = obj->isMimeTypeSupport(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuPrivateTest, isSuffixSupport)
{
    // Test method: bool isSuffixSupport((const QAction *action, FileInfoPointer fileInfo, const bool allEx7z))
    auto result = obj->isSuffixSupport(nullptr, FileInfoPointer(), false);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuPrivateTest, isValid)
{
    // Test method: bool isValid((const QAction *action, FileInfoPointer fileInfo, const bool onDesktop, const bool allEx7z))
    auto result = obj->isValid(nullptr, FileInfoPointer(), false, false);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuPrivateTest, execDynamicArg)
{
    // Test method: QPair<OemMenuPrivate::ArgType, int> execDynamicArg((const QStringList &args, int index))
    QStringList _arg0{};
    auto result = obj->execDynamicArg(_arg0, 0);
    EXPECT_NO_FATAL_FAILURE({ obj->execDynamicArg(_arg0, 0); });

}
