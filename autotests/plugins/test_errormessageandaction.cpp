// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_errormessageandaction.cpp
 * @brief Unit tests for ErrorMessageAndAction methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/errormessageandaction.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class ErrorMessageAndActionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ErrorMessageAndAction();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ErrorMessageAndAction *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ErrorMessageAndActionTest, errorMsg)
{
    // Test method: QString errorMsg((const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error, const bool isTo, const QString &errorMsg, const bool allUsErrorMsg))
    QUrl _arg0{};
    QUrl _arg1{};
    AbstractJobHandler::JobErrorType _arg2{};
    QString _arg4{};
    auto result = obj->errorMsg(_arg0, _arg1, _arg2, false, _arg4, false);
    EXPECT_TRUE(result.isEmpty());

}
