// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalhelper.cpp
 * @brief Unit tests for OpticalHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/opticalhelper.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpticalHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpticalHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalHelperTest, burnDestDevice)
{
    // Test method: QString burnDestDevice((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->burnDestDevice(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalHelperTest, burnFilePath)
{
    // Test method: QString burnFilePath((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->burnFilePath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalHelperTest, burnIsOnDisc)
{
    // Test method: bool burnIsOnDisc((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->burnIsOnDisc(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalHelperTest, createStagingFolder)
{
    // Test method: void createStagingFolder((const QString &dev))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createStagingFolder(_arg0));
}

TEST_F(OpticalHelperTest, discRoot)
{
    // Test method: QUrl discRoot((const QString &dev))
    QString _arg0{};
    auto result = obj->discRoot(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(OpticalHelperTest, findMountPoint)
{
    // Test method: QString findMountPoint((const QString &path))
    QString _arg0{};
    auto result = obj->findMountPoint(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalHelperTest, isBurnEnabled)
{
    // Test bool getter: isBurnEnabled()
    bool result = obj->isBurnEnabled();
    EXPECT_FALSE(result);

}

TEST_F(OpticalHelperTest, localStagingFile)
{
    // Test method: QUrl localStagingFile((QString dev))
    auto result = obj->localStagingFile(QString());
    EXPECT_FALSE(result.isValid());

}

TEST_F(OpticalHelperTest, localStagingRoot)
{
    // Test getter: QUrl localStagingRoot()
    auto result = obj->localStagingRoot();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(OpticalHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OpticalHelperTest, transDiscRootById)
{
    // Test method: QUrl transDiscRootById((const QString &id))
    QString _arg0{};
    auto result = obj->transDiscRootById(_arg0);
    EXPECT_FALSE(result.isValid());

}
