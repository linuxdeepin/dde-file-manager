// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QSignalSpy>

#include "trash.h"
#include "utils/trashhelper.h"
#include "trashdiriterator.h"
#include "trashfilewatcher.h"
#include "utils/trashfilehelper.h"
#include "menus/trashmenuscene.h"

#include <stubext.h>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPTRASH_USE_NAMESPACE
using namespace dfmplugin_trash;

class TestTrashPlugin : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TestTrashPlugin, Initialize)
{
    Trash trash;
    // Just test that the function can be called without crashing
    trash.initialize();
    EXPECT_TRUE(true);
}

TEST_F(TestTrashPlugin, Start)
{
    Trash trash;
    // Just test that the function can be called without crashing
    bool result = trash.start();
    EXPECT_TRUE(result);
}

TEST_F(TestTrashPlugin, Destructor)
{
    Trash *trash = new Trash();
    EXPECT_NE(trash, nullptr);
    delete trash;
    // Test that destructor works without crash
    EXPECT_TRUE(true);
}