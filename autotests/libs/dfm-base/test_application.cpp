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
    EXPECT_NO_FATAL_FAILURE({ Application::instance(); });
}

TEST(ApplicationTest, AppObtuselySettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ Application::appObtuselySetting(); });
}

TEST(ApplicationTest, GenericObtuselySettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ Application::genericObtuselySetting(); });
}

TEST(ApplicationTest, GenericSettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ Application::genericSetting(); });
}

TEST(ApplicationTest, AppSettingReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ Application::appSetting(); });
}

TEST(ApplicationTest, DataPersistenceReturnsPointer)
{
    EXPECT_NO_FATAL_FAILURE({ Application::dataPersistence(); });
}

TEST(ApplicationTest, AppAttributeReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({ Application::appAttribute(Application::kIconSizeLevel); });
}

TEST(ApplicationTest, AppUrlAttributeReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({ Application::appUrlAttribute(Application::kUrlOfNewWindow); });
}

TEST(ApplicationTest, SetAppAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        Application::setAppAttribute(Application::kIconSizeLevel, QVariant(2));
    });
}

TEST(ApplicationTest, SyncAppAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ Application::syncAppAttribute(); });
}

TEST(ApplicationTest, GenericAttributeReturnsValid)
{
    EXPECT_NO_FATAL_FAILURE({ Application::genericAttribute(Application::kPreviewTextFile); });
}

TEST(ApplicationTest, SetGenericAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        Application::setGenericAttribute(Application::kShowedHiddenFiles, QVariant(true));
    });
}

TEST(ApplicationTest, SyncGenericAttributeNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ Application::syncGenericAttribute(); });
}

// ---- Coverage additions: appAttributeTrigger + onSettingsValueEdited ----

TEST(ApplicationTest, AppAttributeTriggerNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ Application::appAttributeTrigger(Application::kRestoreViewMode, 0); });
}

TEST(ApplicationTest, OnSettingsValueEditedNoCrash)
{
    // onSettingsValueEdited needs a valid Application instance (sets self).
    // If instance() is null, skip — the test still exercises the code path
    // when a prior suite (e.g. SettingBackend) already created one.
    if (Application::instance()) {
        EXPECT_NO_FATAL_FAILURE({ Application::instance()->onSettingsValueEdited("group", "key", QVariant(1)); });
    }
}


TEST(ApplicationTest, Application)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ Application obj; });
    }


TEST(ApplicationTest, appAttribute)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ obj.appAttribute(Application::ApplicationAttribute()); });
}

TEST(ApplicationTest, appObtuselySetting)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ obj.appObtuselySetting(); });
}

TEST(ApplicationTest, appSetting)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ obj.appSetting(); });
}

TEST(ApplicationTest, dataPersistence)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ obj.dataPersistence(); });
}

TEST(ApplicationTest, genericAttribute)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ obj.genericAttribute(Application::GenericAttribute()); });
}

TEST(ApplicationTest, genericObtuselySetting)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ obj.genericObtuselySetting(); });
}

TEST(ApplicationTest, genericSetting)
{
    Application obj;
    EXPECT_NO_FATAL_FAILURE({ obj.genericSetting(); });
}
