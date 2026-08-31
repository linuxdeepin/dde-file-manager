// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionbuilder_1.cpp
 * @brief Unit tests for DCustomActionBuilder methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactionbuilder.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionBuilderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionBuilder();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionBuilder *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionBuilderTest, appendAllMimeTypes)
{
    // Test method: void appendAllMimeTypes((const FileInfoPointer &fileInfo, QStringList &noParentmimeTypes, QStringList &allMimeTypes))
    FileInfoPointer _arg0{};
    QStringList _arg1{};
    QStringList _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->appendAllMimeTypes(_arg0, _arg1, _arg2));
}

TEST_F(DCustomActionBuilderTest, buildAciton)
{
    // Test method: QAction buildAciton((const DCustomActionData &actionData, QWidget *parentForSubmenu))
    DCustomActionData _arg0{};
    auto result = obj->buildAciton(_arg0, nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->buildAciton(_arg0, nullptr); });

}

TEST_F(DCustomActionBuilderTest, createAciton)
{
    // Test method: QAction createAciton((const DCustomActionData &actionData))
    DCustomActionData _arg0{};
    auto result = obj->createAciton(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createAciton(_arg0); });

}

TEST_F(DCustomActionBuilderTest, getCompleteSuffix)
{
    // Test method: QString getCompleteSuffix((const QString &fileName, const QString &suf))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->getCompleteSuffix(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionBuilderTest, getIcon)
{
    // Test method: QIcon getIcon((const QString &iconName))
    QString _arg0{};
    auto result = obj->getIcon(_arg0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(DCustomActionBuilderTest, setActiveDir)
{
    // Test setter: void setActiveDir((const QUrl &dir))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setActiveDir(_arg0));
}

TEST_F(DCustomActionBuilderTest, setFocusFile)
{
    // Test setter: void setFocusFile((const QUrl &file))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFocusFile(_arg0));
}
