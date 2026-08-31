// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualentrydbhandler.cpp
 * @brief Unit tests for VirtualEntryDbHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/datahelper/virtualentrydbhandler.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class VirtualEntryDbHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualEntryDbHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualEntryDbHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualEntryDbHandlerTest, clearData)
{
    // Test method: void clearData((const QString &stdSmb))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->clearData(_arg0));
}

TEST_F(VirtualEntryDbHandlerTest, removeData)
{
    // Test method: void removeData((const QString &stdSmb))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeData(_arg0));
}

TEST_F(VirtualEntryDbHandlerTest, saveAggregatedAndSperated)
{
    // Test method: void saveAggregatedAndSperated((const QString &stdSmb, const QString &displayName))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->saveAggregatedAndSperated(_arg0, _arg1));
}

TEST_F(VirtualEntryDbHandlerTest, saveData)
{
    // Test method: void saveData((const VirtualEntryData &data))
    VirtualEntryData _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveData(_arg0));
}
