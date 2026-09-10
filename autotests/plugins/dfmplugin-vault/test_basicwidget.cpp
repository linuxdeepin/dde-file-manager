// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCloseEvent>
#include <QIcon>

#include "stubext.h"

#define private public
#define protected public
#include "views/vaultpropertyview/basicwidget.h"
#undef protected
#undef private

#include <dfm-base/utils/fileutils.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class BasicWidgetTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        widget = new BasicWidget();
    }

    void TearDown() override
    {
        stub.clear();
        delete widget;
    }

    BasicWidget *widget = nullptr;
};

TEST_F(BasicWidgetTest, GetFileSize_DefaultZero)
{
    EXPECT_EQ(widget->getFileSize(), 0);
}

TEST_F(BasicWidgetTest, GetFileCount_DefaultZero)
{
    EXPECT_EQ(widget->getFileCount(), 0);
}

TEST_F(BasicWidgetTest, CloseEvent_NoCrash)
{
    QCloseEvent event;
    EXPECT_NO_FATAL_FAILURE(widget->closeEvent(&event));
}

TEST_F(BasicWidgetTest, SlotFileCountAndSizeChange_UpdatesMembers)
{
    DFMBASE_NAMESPACE::FileScanner::ScanResult result;
    result.totalSize = 1024;
    result.fileCount = 5;
    result.directoryCount = 3;

    EXPECT_NO_FATAL_FAILURE(widget->slotFileCountAndSizeChange(result));
    EXPECT_EQ(widget->fSize, 1024);
    EXPECT_EQ(widget->fCount, 8);
    EXPECT_EQ(widget->getFileSize(), 1024);
    EXPECT_EQ(widget->getFileCount(), 8);
}
