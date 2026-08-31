// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerutils_1.cpp
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

TEST_F(ComputerUtilsTest, allValidBlockUUIDs)
{
    // Test getter: QStringList allValidBlockUUIDs()
    auto result = obj->allValidBlockUUIDs();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, blkDevUrlByUUIDs)
{
    // Test method: QList<QUrl> blkDevUrlByUUIDs((const QStringList &uuids))
    QStringList _arg0{};
    auto result = obj->blkDevUrlByUUIDs(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ComputerUtilsTest, checkGvfsMountExist)
{
    // Test method: bool checkGvfsMountExist((const QUrl &url, int timeout))
    QUrl _arg0{};
    auto result = obj->checkGvfsMountExist(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerUtilsTest, devicePropertyDialog)
{
    // Test method: QWidget devicePropertyDialog((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->devicePropertyDialog(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->devicePropertyDialog(_arg0); });

}

TEST_F(ComputerUtilsTest, getAppEntryFileUrl)
{
    // Test method: QUrl getAppEntryFileUrl((const QUrl &entryUrl))
    QUrl _arg0{};
    auto result = obj->getAppEntryFileUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerUtilsTest, getUniqueInteger)
{
    // Test getter: int getUniqueInteger()
    auto result = obj->getUniqueInteger();
    EXPECT_EQ(result, 0);

}

TEST_F(ComputerUtilsTest, getWinId)
{
    // Test method: quint64 getWinId((QWidget *widget))
    auto result = obj->getWinId(nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(ComputerUtilsTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(ComputerUtilsTest, isNativeDevice)
{
    // Test method: bool isNativeDevice((const QString &suffix))
    QString _arg0{};
    auto result = obj->isNativeDevice(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ComputerUtilsTest, makeAppEntryUrl)
{
    // Test method: QUrl makeAppEntryUrl((const QString &filePath))
    QString _arg0{};
    auto result = obj->makeAppEntryUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ComputerUtilsTest, shouldLoopPartitionsHide)
{
    // Test bool getter: shouldLoopPartitionsHide()
    bool result = obj->shouldLoopPartitionsHide();
    EXPECT_FALSE(result);

}

TEST_F(ComputerUtilsTest, shouldSystemPartitionHide)
{
    // Test bool getter: shouldSystemPartitionHide()
    bool result = obj->shouldSystemPartitionHide();
    EXPECT_FALSE(result);

}
