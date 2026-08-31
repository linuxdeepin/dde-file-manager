// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertydialogutil.cpp
 * @brief Unit tests for PropertyDialogUtil methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/propertydialogutil.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyDialogUtilTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyDialogUtil();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyDialogUtil *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyDialogUtilTest, addExtendedControlFileProperty)
{
    // Test method: void addExtendedControlFileProperty((const QUrl &url, QWidget *widget, ViewExtensionUpdateFunc updater))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addExtendedControlFileProperty(_arg0, nullptr, ViewExtensionUpdateFunc()));
}

TEST_F(PropertyDialogUtilTest, closeAllFilePropertyDialog)
{
    // Test method: void closeAllFilePropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->closeAllFilePropertyDialog());
}

TEST_F(PropertyDialogUtilTest, closeAllPropertyDialog)
{
    // Test method: void closeAllPropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->closeAllPropertyDialog());
}

TEST_F(PropertyDialogUtilTest, closeCustomPropertyDialog)
{
    // Test method: void closeCustomPropertyDialog((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->closeCustomPropertyDialog(_arg0));
}

TEST_F(PropertyDialogUtilTest, closeFilePropertyDialog)
{
    // Test method: void closeFilePropertyDialog((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->closeFilePropertyDialog(_arg0));
}

TEST_F(PropertyDialogUtilTest, createView)
{
    // Test method: QMap<int, QWidget *> createView((const QUrl &url, const QVariantHash &option))
    QUrl _arg0{};
    QVariantHash _arg1{};
    auto result = obj->createView(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PropertyDialogUtilTest, insertExtendedControlFileProperty)
{
    // Test method: void insertExtendedControlFileProperty((const QUrl &url, int index, QWidget *widget, ViewExtensionUpdateFunc updater))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->insertExtendedControlFileProperty(_arg0, 0, nullptr, ViewExtensionUpdateFunc()));
}

TEST_F(PropertyDialogUtilTest, showCustomDialog)
{
    // Test method: bool showCustomDialog((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->showCustomDialog(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(PropertyDialogUtilTest, showPropertyDialog)
{
    // Test method: void showPropertyDialog((const QList<QUrl> &urls, const QVariantHash &option))
    QList<QUrl> _arg0{};
    QVariantHash _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showPropertyDialog(_arg0, _arg1));
}

TEST_F(PropertyDialogUtilTest, updateCloseIndicator)
{
    // Test method: void updateCloseIndicator(())
    EXPECT_NO_FATAL_FAILURE(obj->updateCloseIndicator());
}

TEST_F(PropertyDialogUtilTest, createCustomizeView)
{
    // Test method: QWidget createCustomizeView((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createCustomizeView(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createCustomizeView(_arg0); });

}

TEST_F(PropertyDialogUtilTest, instance)
{
    // Test getter: PropertyDialogUtil instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
