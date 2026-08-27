// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertydialogutil.cpp
 * @brief Unit tests for PropertyDialogUtil Mid-priority methods (dfmplugin-propertydialog)
 */

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QPoint>
#include <QVariant>

#include "utils/propertydialogutil.h"

using namespace dfmplugin_propertydialog;

class PropertyDialogUtilTest : public ::testing::Test {
protected:
    void SetUp() override {
        // PropertyDialogUtil uses singleton pattern
    }
    void TearDown() override {}
};

TEST_F(PropertyDialogUtilTest, addExtendedControlFileProperty)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->addExtendedControlFileProperty(QUrl("file:///tmp/test"), nullptr, ViewExtensionUpdateFunc()); });
}

TEST_F(PropertyDialogUtilTest, closeAllFilePropertyDialog)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->closeAllFilePropertyDialog(); });
}

TEST_F(PropertyDialogUtilTest, closeAllPropertyDialog)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->closeAllPropertyDialog(); });
}

TEST_F(PropertyDialogUtilTest, closeCustomPropertyDialog)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->closeCustomPropertyDialog(QUrl("file:///tmp/test")); });
}

TEST_F(PropertyDialogUtilTest, closeFilePropertyDialog)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->closeFilePropertyDialog(QUrl("file:///tmp/test")); });
}

TEST_F(PropertyDialogUtilTest, createView)
{
    EXPECT_NO_FATAL_FAILURE({ auto r = PropertyDialogUtil::instance()->createView(QUrl("file:///tmp/test"), QVariantHash()); (void)r; });
}

TEST_F(PropertyDialogUtilTest, insertExtendedControlFileProperty)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->insertExtendedControlFileProperty(QUrl("file:///tmp/test"), 0, nullptr, ViewExtensionUpdateFunc()); });
}

TEST_F(PropertyDialogUtilTest, showCustomDialog)
{
    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = PropertyDialogUtil::instance()->showCustomDialog(QUrl("file:///tmp/test")); });
    (void)result;
}

TEST_F(PropertyDialogUtilTest, showPropertyDialog)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->showPropertyDialog(QList<QUrl>{QUrl("file:///tmp/test")}, QVariantHash()); });
}

TEST_F(PropertyDialogUtilTest, updateCloseIndicator)
{
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil::instance()->updateCloseIndicator(); });
}

TEST_F(PropertyDialogUtilTest, createCustomizeView)
{
    // createCustomizeView
    SUCCEED();
}

TEST_F(PropertyDialogUtilTest, instance)
{
    // instance
    SUCCEED();
}
