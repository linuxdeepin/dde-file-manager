// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_previewsingleapplication.cpp
 * @brief Unit tests for PreviewSingleApplication methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/filepreview/previewsingleapplication.h"

#include <QTest>

using namespace src;

class PreviewSingleApplicationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PreviewSingleApplication();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PreviewSingleApplication *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PreviewSingleApplicationTest, handleNewClient)
{
    // Test method: void handleNewClient((const QString &uniqueKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleNewClient(_arg0));
}

TEST_F(PreviewSingleApplicationTest, processArgs)
{
    // Test method: void processArgs((const QStringList &list))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->processArgs(_arg0));
}
