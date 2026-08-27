// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opticalhelper.cpp
 * @brief Unit tests for OpticalHelper Mid-priority methods (dfmplugin-optical)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "utils/opticalhelper.h"

using namespace dfmplugin_optical;

class OpticalHelperTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {}
};

TEST_F(OpticalHelperTest, burnDestDevice)
{
    // Instance method burnDestDevice
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.burnDestDevice(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(OpticalHelperTest, burnFilePath)
{
    // Instance method burnFilePath
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.burnFilePath(QUrl("file:///tmp/test")); (void)r; });
}

TEST_F(OpticalHelperTest, burnIsOnDisc)
{
    // Instance method burnIsOnDisc
    OpticalHelper obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.burnIsOnDisc(QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(OpticalHelperTest, createStagingFolder)
{
    // Instance method createStagingFolder
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ obj.createStagingFolder(QString("test")); });
}

TEST_F(OpticalHelperTest, discRoot)
{
    // Instance method discRoot
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.discRoot(QString("test")); (void)r; });
}

TEST_F(OpticalHelperTest, findMountPoint)
{
    // Instance method findMountPoint
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.findMountPoint(QString("test")); (void)r; });
}

TEST_F(OpticalHelperTest, isBurnEnabled)
{
    // Instance method isBurnEnabled
    OpticalHelper obj;
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = obj.isBurnEnabled(); });
    (void)result;
}

TEST_F(OpticalHelperTest, localStagingFile)
{
    // Instance method localStagingFile
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.localStagingFile(QString("test")); (void)r; });
}

TEST_F(OpticalHelperTest, localStagingRoot)
{
    // Instance method localStagingRoot
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.localStagingRoot(); (void)r; });
}

TEST_F(OpticalHelperTest, scheme)
{
    // Instance method scheme
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.scheme(); (void)r; });
}

TEST_F(OpticalHelperTest, transDiscRootById)
{
    // Instance method transDiscRootById
    OpticalHelper obj;
    EXPECT_NO_FATAL_FAILURE({ auto r = obj.transDiscRootById(QString("test")); (void)r; });
}
