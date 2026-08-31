// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workerpipe.cpp
 * @brief Unit tests for WorkerPipe methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-extractor/libextractor/workerpipe.h"

#include <QTest>

using namespace src;

class WorkerPipeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkerPipe();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkerPipe *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkerPipeTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(WorkerPipeTest, processInputBuffer)
{
    // Test method: void processInputBuffer(())
    EXPECT_NO_FATAL_FAILURE(obj->processInputBuffer());
}

TEST_F(WorkerPipeTest, sendStatus)
{
    // Test method: bool sendStatus((ExtractorStatus status, const QString &filePath, const QByteArray &data))
    QString _arg1{};
    QByteArray _arg2{};
    auto result = obj->sendStatus(ExtractorStatus(), _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(WorkerPipeTest, writePacket)
{
    // Test method: bool writePacket((const QByteArray &packetData))
    QByteArray _arg0{};
    auto result = obj->writePacket(_arg0);
    EXPECT_FALSE(result);

}
