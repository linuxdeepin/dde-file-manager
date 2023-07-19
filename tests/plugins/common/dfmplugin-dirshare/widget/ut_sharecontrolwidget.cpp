// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/common/dfmplugin-dirshare/widget/sharecontrolwidget.h"

#include <gtest/gtest.h>

using namespace dfmplugin_dirshare;

class UT_ShareControlWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        widget = new ShareControlWidget(QUrl::fromLocalFile("/home"));
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete widget;
    }

private:
    stub_ext::StubExt stub;
    ShareControlWidget *widget { nullptr };
};

TEST_F(UT_ShareControlWidget, SetOption)
{
    EXPECT_NO_FATAL_FAILURE(ShareControlWidget::setOption(widget, {}));
}

TEST_F(UT_ShareControlWidget, ValidateShareName) { }
TEST_F(UT_ShareControlWidget, UpdateShare) { }
TEST_F(UT_ShareControlWidget, ShareFolder) { }
TEST_F(UT_ShareControlWidget, UnshareFolder) { }
TEST_F(UT_ShareControlWidget, UpdateWidgetStatus) { }
TEST_F(UT_ShareControlWidget, OnSambapasswordSet) { }
TEST_F(UT_ShareControlWidget, ShowMoreInfo) { }
TEST_F(UT_ShareControlWidget, UserShareOperation) { }
TEST_F(UT_ShareControlWidget, ShowSharePasswordSettingsDialog) { }
