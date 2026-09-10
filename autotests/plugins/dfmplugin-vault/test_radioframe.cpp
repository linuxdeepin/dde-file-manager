// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QPaintEvent>
#include <QRect>

#include "stubext.h"

#include "dfmplugin_vault_global.h"
#include "views/radioframe.h"

DPVAULT_USE_NAMESPACE

class RadioFrameTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        view = new RadioFrame();
        view->resize(200, 100);
    }

    void TearDown() override
    {
        stub.clear();
        delete view;
    }

    RadioFrame *view = nullptr;
};

TEST_F(RadioFrameTest, Constructor_CreatesView)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(RadioFrameTest, PaintEvent_NoCrash)
{
    QRect rect(0, 0, 200, 100);
    QPaintEvent event(rect);
    EXPECT_NO_FATAL_FAILURE(view->paintEvent(&event));
}
