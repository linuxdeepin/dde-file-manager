// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "dfmplugin_trashcore_global.h"
#include "plugins/common/core/dfmplugin-trashcore/trashcore.h"
#include "plugins/common/core/dfmplugin-trashcore/events/trashcoreeventreceiver.h"

#include <dfm-base/base/schemefactory.h>

#include <gtest/gtest.h>

DPTRASHCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_TrashCoreEventReceiver : public testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
    ~UT_TrashCoreEventReceiver() override
    {}
};

TEST_F(UT_TrashCoreEventReceiver, testTrashCoreEventReceiver)
{
    TrashCore core;
    core.initialize();

    TrashCoreEventReceiver *receiver = TrashCoreEventReceiver::instance();
    receiver->handleEmptyTrash(0);

    stub_ext::StubExt stub;
    EXPECT_FALSE(receiver->copyFromFile(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(receiver->copyFromFile(0, {QUrl()}, QUrl(), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_TRUE(receiver->copyFromFile(0, {QUrl("trash:///")}, QUrl(), AbstractJobHandler::JobFlag::kNoHint));

    EXPECT_TRUE(receiver->cutFileFromTrash(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_FALSE(receiver->copyFromFile(0, {QUrl()}, QUrl(), AbstractJobHandler::JobFlag::kNoHint));
    EXPECT_TRUE(receiver->copyFromFile(0, {QUrl("trash:///")}, QUrl(), AbstractJobHandler::JobFlag::kNoHint));
}
