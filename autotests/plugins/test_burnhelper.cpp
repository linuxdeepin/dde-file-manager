// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burnhelper.cpp
 * @brief Unit tests for BurnHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnhelper.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnHelperTest, burnDestDevice)
{
    // Test method: QString burnDestDevice((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->burnDestDevice(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BurnHelperTest, burnFilePath)
{
    // Test method: QString burnFilePath((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->burnFilePath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BurnHelperTest, discDataGroup)
{
    // Test getter: QList<QVariantMap> discDataGroup()
    auto result = obj->discDataGroup();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BurnHelperTest, fromBurnFile)
{
    // Test method: QUrl fromBurnFile((const QString &dev))
    QString _arg0{};
    auto result = obj->fromBurnFile(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BurnHelperTest, isBurnEnabled)
{
    // Test bool getter: isBurnEnabled()
    bool result = obj->isBurnEnabled();
    EXPECT_FALSE(result);

}

TEST_F(BurnHelperTest, localFileInfoList)
{
    // Test method: QFileInfoList localFileInfoList((const QString &path))
    QString _arg0{};
    auto result = obj->localFileInfoList(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BurnHelperTest, localFileInfoListRecursive)
{
    // Test method: QFileInfoList localFileInfoListRecursive((const QString &path, QDir::Filters filters))
    QString _arg0{};
    auto result = obj->localFileInfoListRecursive(_arg0, QDir::Filters());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BurnHelperTest, localStagingFile)
{
    // Test method: QUrl localStagingFile((const QUrl &dest))
    QUrl _arg0{};
    auto result = obj->localStagingFile(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BurnHelperTest, mapStagingFilesPath)
{
    // Test method: void mapStagingFilesPath((const QList<QUrl> &srcList, const QList<QUrl> &targetList))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->mapStagingFilesPath(_arg0, _arg1));
}

TEST_F(BurnHelperTest, parseXorrisoErrorMessage)
{
    // Test method: QString parseXorrisoErrorMessage((const QStringList &msg))
    QStringList _arg0{};
    auto result = obj->parseXorrisoErrorMessage(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BurnHelperTest, showOpticalBlankConfirmationDialog)
{
    // Test getter: int showOpticalBlankConfirmationDialog()
    auto result = obj->showOpticalBlankConfirmationDialog();
    EXPECT_EQ(result, 0);

}

TEST_F(BurnHelperTest, showOpticalImageOpSelectionDialog)
{
    // Test getter: int showOpticalImageOpSelectionDialog()
    auto result = obj->showOpticalImageOpSelectionDialog();
    EXPECT_EQ(result, 0);

}

TEST_F(BurnHelperTest, updateBurningStateToPersistence)
{
    // Test method: void updateBurningStateToPersistence((const QString &id, const QString &dev, bool working))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateBurningStateToPersistence(_arg0, _arg1, false));
}
