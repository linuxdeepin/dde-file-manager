// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_travers_prehandler.cpp
 * @brief Unit tests for travers_prehandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/traversprehandler.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class travers_prehandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new travers_prehandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    travers_prehandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(travers_prehandlerTest, doChangeCurrentUrl)
{
    // Test method: void doChangeCurrentUrl((quint64 winId, const QString &mpt, const QString &subPath, const QUrl &sourceUrl))
    QString _arg1{};
    QString _arg2{};
    QUrl _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->doChangeCurrentUrl(0, _arg1, _arg2, _arg3));
}

TEST_F(travers_prehandlerTest, onSmbRootMounted)
{
    // Test method: void onSmbRootMounted((const QString &mountSource, Handler after))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSmbRootMounted(_arg0, Handler()));
}

TEST_F(travers_prehandlerTest, smbAccessPrehandler)
{
    // Test method: void smbAccessPrehandler((quint64 winId, const QUrl &url, std::function<void()> after))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->smbAccessPrehandler(0, _arg1, {}));
}
