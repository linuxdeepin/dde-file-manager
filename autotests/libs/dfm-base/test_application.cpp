// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_application.cpp
 * @brief Unit tests for Application static config API (application.cpp)
 */

#include <gtest/gtest.h>
#include <QVariant>
#include <QUrl>
#include <QString>

#include <dfm-base/base/application/application.h>

using namespace dfmbase;

TEST(ApplicationTest, InstanceReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::instance(); });
}

TEST(ApplicationTest, AppObtuselySettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::appObtuselySetting(); });
}

TEST(ApplicationTest, GenericObtuselySettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::genericObtuselySetting(); });
}

TEST(ApplicationTest, GenericSettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::genericSetting(); });
}

TEST(ApplicationTest, AppSettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::appSetting(); });
}

TEST(ApplicationTest, DataPersistenceReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::dataPersistence(); });
}

TEST(ApplicationTest, AppAttributeReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)Application::appAttribute(Application::kIconSizeLevel);
    });
}

TEST(ApplicationTest, AppUrlAttributeReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)Application::appUrlAttribute(Application::kUrlOfNewWindow);
    });
}

TEST(ApplicationTest, SetAppAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        Application::setAppAttribute(Application::kIconSizeLevel, QVariant(2));
    });
}

TEST(ApplicationTest, SyncAppAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::syncAppAttribute(); });
}

TEST(ApplicationTest, GenericAttributeReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)Application::genericAttribute(Application::kPreviewTextFile);
    });
}

TEST(ApplicationTest, SetGenericAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        Application::setGenericAttribute(Application::kShowedHiddenFiles, QVariant(true));
    });
}

TEST(ApplicationTest, SyncGenericAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ (void)Application::syncGenericAttribute(); });
}
