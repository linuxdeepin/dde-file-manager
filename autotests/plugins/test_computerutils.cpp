// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerutils.cpp
 * @brief Unit tests for ComputerUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/computerutils.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerUtilsTest, allPreDefineItemCustomDatas)
{
    // Test getter: QList<QVariantMap> allPreDefineItemCustomDatas()
    auto result = obj->allPreDefineItemCustomDatas();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, deviceTypeInfo)
{
    // Test method: QString deviceTypeInfo((DFMEntryFileInfoPointer info))
    auto result = obj->deviceTypeInfo(DFMEntryFileInfoPointer());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, getBlockDevIdByUrl)
{
    // Test method: QString getBlockDevIdByUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getBlockDevIdByUrl(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, getProtocolDevIdByUrl)
{
    // Test method: QString getProtocolDevIdByUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->getProtocolDevIdByUrl(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, isPresetSuffix)
{
    // Test method: bool isPresetSuffix((const QString &suffix))
    QString _arg0{};
    auto result = obj->isPresetSuffix(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerUtilsTest, makeBlockDevUrl)
{
    // Test method: QUrl makeBlockDevUrl((const QString &id))
    QString _arg0{};
    auto result = obj->makeBlockDevUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerUtilsTest, makeBurnUrl)
{
    // Test method: QUrl makeBurnUrl((const QString &id))
    QString _arg0{};
    auto result = obj->makeBurnUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerUtilsTest, makeLocalUrl)
{
    // Test method: QUrl makeLocalUrl((const QString &path))
    QString _arg0{};
    auto result = obj->makeLocalUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerUtilsTest, makeProtocolDevUrl)
{
    // Test method: QUrl makeProtocolDevUrl((const QString &id))
    QString _arg0{};
    auto result = obj->makeProtocolDevUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerUtilsTest, menuSceneName)
{
    // Test getter: QString menuSceneName()
    auto result = obj->menuSceneName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(ComputerUtilsTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, setCursorState)
{
    // Test setter: void setCursorState((bool busy))
    EXPECT_NO_FATAL_FAILURE(obj->setCursorState(false));
}

TEST_F(ComputerUtilsTest, sortItem)
{
    // Test method: bool sortItem((DFMEntryFileInfoPointer a, DFMEntryFileInfoPointer b))
    auto result = obj->sortItem(DFMEntryFileInfoPointer(), DFMEntryFileInfoPointer());
    EXPECT_FALSE(result);

}
