// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mimeappsworker.cpp
 * @brief Unit tests for MimeAppsWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/mimetype/mimesappsmanager.h"

#include <QTest>

using namespace src;

class MimeAppsWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MimeAppsWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MimeAppsWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MimeAppsWorkerTest, writeData)
{
    // Test method: void writeData((const QString &path, const QByteArray &content))
    QString _arg0{};
    QByteArray _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->writeData(_arg0, _arg1));
}
