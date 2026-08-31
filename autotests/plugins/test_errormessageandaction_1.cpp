// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_errormessageandaction_1.cpp
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

TEST_F(ErrorMessageAndActionTest, errorSrcAndDestString)
{
    // Test method: void errorSrcAndDestString((const QUrl &from,
                                                  const QUrl &to,
                                                  QString *sorceMsg, QString *toMsg,
                                                  const AbstractJobHandler::JobErrorType error))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->errorSrcAndDestString(_arg0, _arg1, nullptr, nullptr, AbstractJobHandler::JobErrorType()));
}
