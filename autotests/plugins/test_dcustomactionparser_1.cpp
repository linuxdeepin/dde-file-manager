// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dcustomactionparser_1.cpp
 * @brief Unit tests for DCustomActionParser methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenu/dcustomactionparser.h"

#include <QTest>

using namespace dfmplugin_menu;

class DCustomActionParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DCustomActionParser();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DCustomActionParser *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DCustomActionParserTest, actionFileInfos)
{
    // Test method: bool actionFileInfos((FileBasicInfos &basicInfo, QSettings &actionSetting))
    FileBasicInfos _arg0{};
    QSettings _arg1{};
    auto result = obj->actionFileInfos(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(DCustomActionParserTest, delayRefresh)
{
    // Test method: void delayRefresh(())
    EXPECT_NO_FATAL_FAILURE(obj->delayRefresh());
}

TEST_F(DCustomActionParserTest, getActionFiles)
{
    // Test method: QList<DCustomActionEntry> getActionFiles((bool onDesktop))
    auto result = obj->getActionFiles(false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DCustomActionParserTest, getValue)
{
    // Test method: QVariant getValue((QSettings &actionSetting, const QString &group, const QString &key))
    QSettings _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->getValue(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result.isValid());

}

TEST_F(DCustomActionParserTest, initHash)
{
    // Test method: void initHash(())
    EXPECT_NO_FATAL_FAILURE(obj->initHash());
}

TEST_F(DCustomActionParserTest, isActionShouldShow)
{
    // Test method: bool isActionShouldShow((const QStringList &notShowInList, bool onDesktop))
    QStringList _arg0{};
    auto result = obj->isActionShouldShow(_arg0, false);
    EXPECT_FALSE(result);

}
