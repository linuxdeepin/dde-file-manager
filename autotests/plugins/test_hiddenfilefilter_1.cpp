// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_hiddenfilefilter_1.cpp
 * @brief Unit tests for HiddenFileFilter methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/filters/hiddenfilefilter.h"

#include <QTest>

using namespace ddplugin_organizer;

class HiddenFileFilterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new HiddenFileFilter();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    HiddenFileFilter *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(HiddenFileFilterTest, HiddenFileFilter)
{
    // Test constructor: HiddenFileFilter(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(HiddenFileFilterTest, acceptInsert)
{
    // Test method: bool acceptInsert((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->acceptInsert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HiddenFileFilterTest, acceptRename)
{
    // Test method: bool acceptRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->acceptRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(HiddenFileFilterTest, acceptUpdate)
{
    // Test method: bool acceptUpdate((const QUrl &url, const QVector<int> &roles))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->acceptUpdate(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(HiddenFileFilterTest, hiddenFlagChanged)
{
    // Test method: void hiddenFlagChanged((bool showHidden))
    EXPECT_NO_FATAL_FAILURE(obj->hiddenFlagChanged(false));
}

TEST_F(HiddenFileFilterTest, insertFilter)
{
    // Test method: bool insertFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->insertFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(HiddenFileFilterTest, refreshModel)
{
    // Test method: void refreshModel(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshModel());
}

TEST_F(HiddenFileFilterTest, renameFilter)
{
    // Test method: bool renameFilter((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->renameFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(HiddenFileFilterTest, showHiddenFiles)
{
    // Test bool getter: showHiddenFiles()
    bool result = obj->showHiddenFiles();
    EXPECT_FALSE(result);

}

TEST_F(HiddenFileFilterTest, updateFilter)
{
    // Test method: bool updateFilter((const QUrl &url, const QVector<int> &roles))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->updateFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(HiddenFileFilterTest, updateFlag)
{
    // Test method: void updateFlag(())
    EXPECT_NO_FATAL_FAILURE(obj->updateFlag());
}

TEST_F(HiddenFileFilterTest, HiddenFileFilter_Destructor)
{
    // Test method:  ~HiddenFileFilter(())
    EXPECT_NO_FATAL_FAILURE({ HiddenFileFilter *tmp = new HiddenFileFilter(); delete tmp; });
}
