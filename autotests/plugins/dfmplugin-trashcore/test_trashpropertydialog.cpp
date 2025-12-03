// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

#include "stubext.h"
#include "views/trashpropertydialog.h"
#include "utils/trashcorehelper.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>

using namespace dfmplugin_trashcore;

class TrashPropertyDialogTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(TrashPropertyDialogTest, Constructor_Basic)
{
    TrashPropertyDialog dialog;
    EXPECT_NO_THROW(dialog.windowTitle().isEmpty());
}

TEST_F(TrashPropertyDialogTest, CalculateSize_Basic)
{
    TrashPropertyDialog dialog;
    // This method internally calls TrashCoreHelper::calculateTrashRoot
    bool calculateCalled = false;
    stub.set_lamda(&TrashCoreHelper::calculateTrashRoot, [&calculateCalled]() -> std::pair<qint64, int> {
        calculateCalled = true;
        return std::make_pair<qint64, int>(0, 0);
    });
    
    dialog.calculateSize();
    EXPECT_TRUE(calculateCalled);
}

TEST_F(TrashPropertyDialogTest, ShowEvent_Basic)
{
    TrashPropertyDialog dialog;
    // Just ensure it doesn't crash
    QShowEvent event;
    EXPECT_NO_THROW(dialog.showEvent(&event));
}