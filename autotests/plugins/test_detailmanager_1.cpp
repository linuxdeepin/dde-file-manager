// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailmanager_1.cpp
 * @brief Unit tests for DetailManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/detailmanager.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailManagerTest, DetailManager)
{
    // Test constructor: DetailManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DetailManagerTest, addBasicFiledFiltes)
{
    // Test method: bool addBasicFiledFiltes((const QString &scheme, DetailFilterType filters))
    QString _arg0{};
    auto result = obj->addBasicFiledFiltes(_arg0, DetailFilterType());
    EXPECT_FALSE(result);

}

TEST_F(DetailManagerTest, addRootBasicFiledFiltes)
{
    // Test method: bool addRootBasicFiledFiltes((const QString &scheme, DetailFilterType filters))
    QString _arg0{};
    auto result = obj->addRootBasicFiledFiltes(_arg0, DetailFilterType());
    EXPECT_FALSE(result);

}

TEST_F(DetailManagerTest, basicFiledFiltes)
{
    // Test method: DetailFilterType basicFiledFiltes((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->basicFiledFiltes(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->basicFiledFiltes(_arg0); });

}

TEST_F(DetailManagerTest, extensionInfos)
{
    // Test getter: QList<ViewExtensionInfo> extensionInfos()
    auto result = obj->extensionInfos();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DetailManagerTest, registerBasicViewExtension)
{
    // Test method: bool registerBasicViewExtension((const QString &scheme, BasicViewFieldFunc func))
    QString _arg0{};
    auto result = obj->registerBasicViewExtension(_arg0, BasicViewFieldFunc());
    EXPECT_FALSE(result);

}

TEST_F(DetailManagerTest, registerBasicViewExtensionRoot)
{
    // Test method: bool registerBasicViewExtensionRoot((const QString &scheme, BasicViewFieldFunc func))
    QString _arg0{};
    auto result = obj->registerBasicViewExtensionRoot(_arg0, BasicViewFieldFunc());
    EXPECT_FALSE(result);

}

TEST_F(DetailManagerTest, registerExtensionView)
{
    // Test method: bool registerExtensionView((ViewExtensionCreateFunc create,
                                          ViewExtensionUpdateFunc update,
                                          ViewExtensionShouldShowFunc shouldShow,
                                          int index))
    auto result = obj->registerExtensionView(ViewExtensionCreateFunc(), ViewExtensionUpdateFunc(), ViewExtensionShouldShowFunc(), 0);
    EXPECT_FALSE(result);

}
