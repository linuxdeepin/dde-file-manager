// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_FILEUTILS
#define UT_FILEUTILS
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/standardpaths.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_FileUtils : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

TEST_F(UT_FileUtils, testIsContainProhibitPath)
{
    QList<QUrl> pathUrls;
    pathUrls << QUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());

    EXPECT_EQ(true, FileUtils::isContainProhibitPath(pathUrls));

    QList<QUrl> tempUrls;
    tempUrls << QUrl("/usr/test");
    EXPECT_EQ(false, FileUtils::isContainProhibitPath(tempUrls));

    tempUrls << QUrl("/usr/test")
             << QUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    EXPECT_EQ(true, FileUtils::isContainProhibitPath(tempUrls));
}

#endif
