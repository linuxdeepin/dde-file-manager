// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gioemblemworker.cpp
 * @brief Unit tests for GioEmblemWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/emblemhelper.h"

#include <QTest>

using namespace dfmplugin_emblem;

class GioEmblemWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new GioEmblemWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    GioEmblemWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(GioEmblemWorkerTest, getGioEmblems)
{
    // Test method: QMap<int, QIcon> getGioEmblems((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->getGioEmblems(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(GioEmblemWorkerTest, onClear)
{
    // Test method: void onClear(())
    EXPECT_NO_FATAL_FAILURE(obj->onClear());
}

TEST_F(GioEmblemWorkerTest, parseEmblemString)
{
    // Test method: bool parseEmblemString((QIcon *emblem, QString &pos, const QString &emblemStr))
    QString _arg1{};
    QString _arg2{};
    auto result = obj->parseEmblemString(nullptr, _arg1, _arg2);
    EXPECT_FALSE(result);

}
