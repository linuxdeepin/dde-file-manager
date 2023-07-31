// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dfmplugin_trashcore_global.h"
#include "plugins/common/core/dfmplugin-trashcore/trashcore.h"
#include "plugins/common/core/dfmplugin-trashcore/events/trashcoreeventsender.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>

DPTRASHCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_TrashCoreEventSender : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
    ~UT_TrashCoreEventSender() override
    {}
};

TEST_F(UT_TrashCoreEventSender, testTrashCoreEventSender)
{
    TrashCore core;
    core.initialize();

    stub_ext::StubExt stub;
    stub.set_lamda(VADDR(LocalFileWatcher,startWatcher), []{ __DBG_STUB_INVOKE__ return true;});
    TrashCoreEventSender *sender = TrashCoreEventSender::instance();

    bool empty = FileUtils::trashIsEmpty();
    sender->isEmpty = empty;
    sender->sendTrashStateChangedDel();
    EXPECT_EQ(empty, sender->isEmpty);

    stub.set_lamda(&FileUtils::trashIsEmpty, []{ __DBG_STUB_INVOKE__ return true;});
    sender->isEmpty = false;
    sender->sendTrashStateChangedDel();
    EXPECT_EQ(true, sender->isEmpty);
    sender->sendTrashStateChangedAdd();

    stub.set_lamda(&FileUtils::trashIsEmpty, []{ __DBG_STUB_INVOKE__ return false;});
    sender->isEmpty = true;
    sender->sendTrashStateChangedDel();
    EXPECT_EQ(false, sender->isEmpty);
    sender->sendTrashStateChangedAdd();
}
