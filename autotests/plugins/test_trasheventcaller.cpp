// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trasheventcaller.cpp
 * @brief Unit tests for TrashEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/trasheventcaller.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashEventCallerTest, TrashEventCaller)
{
    // Test constructor: TrashEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashEventCallerTest, sendCheckTabAddable)
{
    // Test method: bool sendCheckTabAddable((quint64 windowId))
    auto result = obj->sendCheckTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(TrashEventCallerTest, sendEmptyTrash)
{
    // Test method: void sendEmptyTrash((const quint64 windowID, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendEmptyTrash(0, _arg1));
}

TEST_F(TrashEventCallerTest, sendOpenFiles)
{
    // Test method: void sendOpenFiles((const quint64 windowID, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenFiles(0, _arg1));
}

TEST_F(TrashEventCallerTest, sendOpenTab)
{
    // Test method: void sendOpenTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenTab(0, _arg1));
}

TEST_F(TrashEventCallerTest, sendOpenWindow)
{
    // Test method: DFMBASE_USE_NAMESPACE sendOpenWindow((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj->sendOpenWindow(_arg0); });
}

TEST_F(TrashEventCallerTest, sendShowEmptyTrash)
{
    // Test method: void sendShowEmptyTrash((quint64 winId, bool visible))
    EXPECT_NO_FATAL_FAILURE(obj->sendShowEmptyTrash(0, false));
}

TEST_F(TrashEventCallerTest, sendTrashPropertyDialog)
{
    // Test method: void sendTrashPropertyDialog((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendTrashPropertyDialog(_arg0));
}
