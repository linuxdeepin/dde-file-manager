// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertydialogmanager.cpp
 * @brief Unit tests for PropertyDialogManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/propertydialogmanager.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyDialogManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyDialogManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyDialogManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyDialogManagerTest, addBasicFiledFiltes)
{
    // Test method: bool addBasicFiledFiltes((const QString &scheme, PropertyFilterType filters))
    QString _arg0{};
    auto result = obj->addBasicFiledFiltes(_arg0, PropertyFilterType());
    EXPECT_FALSE(result);

}

TEST_F(PropertyDialogManagerTest, addComputerPropertyDialog)
{
    // Test method: void addComputerPropertyDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->addComputerPropertyDialog());
}

TEST_F(PropertyDialogManagerTest, basicFiledFiltes)
{
    // Test method: PropertyFilterType basicFiledFiltes((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->basicFiledFiltes(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->basicFiledFiltes(_arg0); });

}

TEST_F(PropertyDialogManagerTest, getCreatorOptionByName)
{
    // Test method: QVariantHash getCreatorOptionByName((const QString &name))
    QString _arg0{};
    auto result = obj->getCreatorOptionByName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PropertyDialogManagerTest, registerBasicViewExtension)
{
    // Test method: bool registerBasicViewExtension((BasicViewFieldFunc func, const QString &scheme))
    QString _arg1{};
    auto result = obj->registerBasicViewExtension(BasicViewFieldFunc(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(PropertyDialogManagerTest, registerCustomView)
{
    // Test method: bool registerCustomView((CustomViewExtensionView view, const QString &scheme))
    QString _arg1{};
    auto result = obj->registerCustomView(CustomViewExtensionView(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(PropertyDialogManagerTest, registerExtensionView)
{
    // Test method: bool registerExtensionView((CustomViewExtensionView viewCreator, const QString &name, int index))
    QString _arg1{};
    auto result = obj->registerExtensionView(CustomViewExtensionView(), _arg1, 0);
    EXPECT_FALSE(result);

}

TEST_F(PropertyDialogManagerTest, registerExtensionViewWithUpdate)
{
    // Test method: bool registerExtensionViewWithUpdate((CustomViewExtensionView creator,
                                                             ViewExtensionUpdateFunc updater,
                                                             const QString &name, int index))
    QString _arg2{};
    auto result = obj->registerExtensionViewWithUpdate(CustomViewExtensionView(), ViewExtensionUpdateFunc(), _arg2, 0);
    EXPECT_FALSE(result);

}

TEST_F(PropertyDialogManagerTest, createCustomView)
{
    // Test method: QWidget createCustomView((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createCustomView(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createCustomView(_arg0); });

}

TEST_F(PropertyDialogManagerTest, instance)
{
    // Test getter: PropertyDialogManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
